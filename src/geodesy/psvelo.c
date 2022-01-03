/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Author:	G. Patau, IGPG, w/ Kurt Feigl
 * Date:	1-JAN-2010
 * Version:	6 API
 *    Copyright (c) 1996-2012 by G. Patau, then donated to the GMT project
 *    by G. Patau upon her retirement from IGPG
 *
 * Roots:		based on psxy.c
 * Adapted to version 3.3 by Genevieve Patau (25 June 1999)
 * Last modified : 18 February 2000.  Ported to GMT 5 by P. Wessel in 2013.
 * Updated 3 March 2021 to add color enhancements by P. Wessel
 *
 * Brief synopsis: psvelo will read coordinates and plot geodetic symbols
 * such as velocity ellipses, strain crosses, or strain wedges on maps.
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"psvelo"
#define THIS_MODULE_MODERN_NAME	"velo"
#define THIS_MODULE_LIB		"geodesy"
#define THIS_MODULE_PURPOSE	"Plot velocity vectors, crosses, anisotropy bars and wedges"
#define THIS_MODULE_KEYS	"<D{,>X}"
#define THIS_MODULE_NEEDS	"Jd"
#define THIS_MODULE_OPTIONS "-:>BJKOPRUVXYdehipqt" GMT_OPT("c")

#define CINE 1
#define ANISO 2
#define WEDGE 3
#define CROSS 4

#define DEFAULT_FONTSIZE	9.0	/* In points */

#define READ_ELLIPSE	0
#define READ_ROTELLIPSE	1
#define READ_ANISOTROPY	2
#define READ_WEDGE	4
#define READ_CROSS	8

/* parameters for writing text */
#define ANGLE		0.0
#define FORM		0

/* Control structure for psvelo */

struct PSVELO_CTRL {
	struct PSVELO_A {	/* -A */
		bool active;
		struct GMT_SYMBOL S;
	} A;
	struct PSVELO_C {	/* -C<cpt> */
		bool active;
		char *file;
	} C;
	struct PSVELO_D {	/* -D */
		bool active;
		double scale;
	} D;
 	struct PSVELO_E {	/* -E<fill> */
		bool active;
		struct GMT_FILL fill;
	} E;
 	struct PSVELO_G {	/* -G<fill> */
		bool active;
		struct GMT_FILL fill;
	} G;
	struct PSVELO_H {	/* -H read overall scaling factor for symbol size and pen width */
		bool active;
		unsigned int mode;
		double value;
	} H;
	struct PSVELO_I {	/* -I[<intensity>] */
		bool active;
		unsigned int mode;	/* 0 if constant, 1 if read from file */
		double value;
	} I;
	struct PSVELO_L {	/* -L[<pen>] */
		bool active;
		bool error_pen;
		struct GMT_PEN pen;
	} L;
	struct PSVELO_N {	/* -N */
		bool active;
	} N;
	struct PSVELO_S {	/* -S<symbol> */
		bool active;
		bool read;	/* True if no scale given; must be first column after the required ones */
		int symbol;
		unsigned int readmode;
		unsigned int n_cols;
		double scale, wedge_amp, conrad;
		double confidence;
		struct GMT_FILL fill;
		struct GMT_FONT font;
	} S;
	struct PSVELO_W {	/* -W<pen> */
		bool active;
		struct GMT_PEN pen;
	} W;
	struct PSVELO_Z {	/* -Z */
		bool active;
		unsigned int mode;
		unsigned int item;
	} Z;
};

enum Psvelo_scaletype {
	PSVELO_READ_SCALE	= 0,
	PSVELO_CONST_SCALE	= 1};

enum psvelo_types {
	PSVELO_G_FILL = 0,
	PSVELO_E_FILL = 1,
	PSVELO_V_MAG	= 0,
	PSVELO_V_EAST,
	PSVELO_V_NORTH,
	PSVELO_R_MAG,
	PSVELO_V_USER
};

/* Content of old utilvelo.c is here */

#define squared(x) ((x) * (x))
#define EPSIL 0.0001

/************************************************************************/
GMT_LOCAL void psvelo_get_trans (struct GMT_CTRL *GMT, double slon, double slat, double *t11, double *t12, double *t21, double *t22) {
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
	/* COMMENT BY PW: Fails as provided if slat > 89.0 and for projection that
	 * gives the same x-coordinates for two different longitudes, as might happen
	 * at the N or S pole.  Some minor protections were added below to handle this.
	 */

	/* LOCAL VARIABLES */
	double su, sv, udlat, vdlat, udlon, vdlon, dudlat, dvdlat, dudlon, dvdlon, dl;
	int flip = 0;

	/* how much does x,y change for a 1 degree change in lon,lon ? */
	gmt_geo_to_xy (GMT, slon,     slat,     &su,    &sv );
	if ((slat+1.0) >= 90.0) {	/* PW: Must do something different at/near NP */
	        gmt_geo_to_xy (GMT, slon,     slat-1.0, &udlat, &vdlat);
		flip = 1;
	}
	else
		gmt_geo_to_xy (GMT, slon,     slat+1.0, &udlat, &vdlat);
	gmt_geo_to_xy (GMT, slon+1.0, slat    , &udlon, &vdlon);

	/* Compute dudlat, dudlon, dvdlat, dvdlon */
	dudlat = udlat - su;
	dvdlat = vdlat - sv;
	dudlon = udlon - su;
	dvdlon = vdlon - sv;
	if (flip) {	/* Fix what we did above */
		dudlat = -dudlat;
		dvdlat = -dvdlat;
	}

	/* Make unit vectors for the long (e/x) and lat (n/y) */
	/* to construct local transformation matrix */

	dl = sqrt (dudlon*dudlon + dvdlon*dvdlon);
	*t11 = (dl == 0.0) ? 0.0 : dudlon/dl;
	*t21 = (dl == 0.0) ? 0.0 : dvdlon/dl;

	dl = sqrt (dudlat*dudlat + dvdlat*dvdlat);
	*t12 = (dl == 0.0) ? 0.0 : dudlat/dl;
	*t22 = (dl == 0.0) ? 0.0 : dvdlat/dl;
}

GMT_LOCAL void psvelo_transform_local (double x0, double y0, double dxp, double dyp, double scale, double t11, double t12, double t21, double t22, double *x1, double *y1) {
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

GMT_LOCAL void psvelo_trace_arrow (struct GMT_CTRL *GMT, double slon, double slat, double dxp, double dyp, double scale, double *x1, double *y1, double *x2, double *y2) {
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
	psvelo_get_trans (GMT, slon, slat, &t11, &t12, &t21, &t22);

	/* map start of arrow from lat, lon to x, y */
	gmt_geo_to_xy (GMT, slon, slat, &xt, &yt);

	/* perform the transformation */
	psvelo_transform_local (xt, yt, dxp, dyp, scale, t11, t12, t21, t22, x2, y2);

	/* return values */

	*x1 = xt;
	*y1 = yt;
}

GMT_LOCAL void psvelo_trace_ellipse (double angle, double major, double minor, int npoints, double *x, double *y) {
	/* Given specs for an ellipse, return it in x,y */
	double phi = 0.0, sd, cd, s, c;
	int i;

	sincosd (angle, &sd, &cd);

	for (i = 0; i < 360; i++) {
		sincos (phi, &s, &c);
		*x++ = major * c * cd - minor * s * sd;
		*y++ = major * c * sd + minor * s * cd;
		phi += M_PI*2.0/(npoints-2);
	}
}

GMT_LOCAL void psvelo_ellipse_convert (double sigx, double sigy, double rho, double conrad, double *eigen1, double *eigen2, double *ang) {
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

GMT_LOCAL void psvelo_paint_ellipse (struct GMT_CTRL *GMT, double x0, double y0, double angle, double major, double minor, double scale, double t11, double t12, double t21, double t22, int polygon, struct GMT_FILL *fill, int outline) {
	/* Make an ellipse at center x0,y0  */
#define NPOINTS_ELLIPSE 362

	int npoints = NPOINTS_ELLIPSE, i;
	/* relative to center of ellipse */
	double dxe[NPOINTS_ELLIPSE],dye[NPOINTS_ELLIPSE];
	/* absolute paper coordinates */
	double axe[NPOINTS_ELLIPSE],aye[NPOINTS_ELLIPSE];

	psvelo_trace_ellipse (angle, major, minor, npoints, dxe, dye);

	for (i = 0; i < npoints - 2; i++) psvelo_transform_local (x0, y0, dxe[i], dye[i], scale, t11, t12, t21, t22, &axe[i], &aye[i]);
	if (polygon) {
		gmt_setfill (GMT, fill, outline);
		PSL_plotpolygon (GMT->PSL, axe, aye, npoints - 2);
	}
	else
		PSL_plotline (GMT->PSL, axe, aye, npoints - 2, PSL_MOVE|PSL_STROKE|PSL_CLOSE);
}

/************************************************************************/
GMT_LOCAL int psvelo_trace_cross (struct GMT_CTRL *GMT, double slon, double slat, double eps1, double eps2, double theta, double sscale, double v_width, double h_length, double h_width, double vector_shape, int outline, struct GMT_PEN *pen) {
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
	double dx, dy, x1, x2, y1, y2, hl, hw, vw, s, c, dim[PSL_MAX_DIMS];
	gmt_M_unused(outline);

	gmt_M_memset (dim, PSL_MAX_DIMS, double);
	gmt_setpen (GMT, pen);			/* Pen for segment line */
	PSL_setfill (GMT->PSL, pen->rgb, 0);	/* Same color for arrow head fill with no outline */
	sincosd (theta, &s, &c);

	/*  extension component */
	dx =  eps1 * c;
	dy = -eps1 * s;

	/* arrow is outward from slat,slon */
	psvelo_trace_arrow (GMT, slon, slat, dx, dy, sscale, &x1, &y1, &x2, &y2);

	if (eps1 < 0.0) {
		gmt_M_double_swap (x1, x2);
		gmt_M_double_swap (y1, y2);
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

	dim[PSL_VEC_XTIP] = x2;
	dim[PSL_VEC_YTIP] = y2;
	dim[PSL_VEC_TAIL_WIDTH] = vw;
	dim[PSL_VEC_HEAD_LENGTH] = hl;
	dim[PSL_VEC_HEAD_WIDTH] = hw;
	dim[PSL_VEC_HEAD_SHAPE] = vector_shape, dim[PSL_VEC_STATUS] = PSL_VEC_END | PSL_VEC_FILL;
	PSL_plotsymbol (GMT->PSL, x1, y1, dim, PSL_VECTOR);

	/* second, extensional arrow in opposite direction */

	psvelo_trace_arrow (GMT, slon, slat, -dx, -dy, sscale, &x1, &y1, &x2, &y2);

	if (eps1 < 0.0) {
		gmt_M_double_swap (x1, x2);
		gmt_M_double_swap (y1, y2);
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

	dim[PSL_VEC_XTIP] = x2;
	dim[PSL_VEC_YTIP] = y2;
	dim[PSL_VEC_TAIL_WIDTH] = vw;
	dim[PSL_VEC_HEAD_LENGTH] = hl;
	dim[PSL_VEC_HEAD_WIDTH] = hw;
	PSL_plotsymbol (GMT->PSL, x1, y1, dim, PSL_VECTOR);

	/* compression component */
	dx = eps2 * s;
	dy = eps2 * c;
	dim[PSL_VEC_STATUS] = PSL_VEC_BEGIN | PSL_VEC_FILL;
	psvelo_trace_arrow (GMT, slon, slat, dx, dy, sscale, &x1, &y1, &x2, &y2);

	if (eps2 > 0.0) {
		gmt_M_double_swap (x1, x2);
		gmt_M_double_swap (y1, y2);
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

	dim[PSL_VEC_XTIP] = x2;
	dim[PSL_VEC_YTIP] = y2;
	dim[PSL_VEC_TAIL_WIDTH] = vw;
	dim[PSL_VEC_HEAD_LENGTH] = hl;
	dim[PSL_VEC_HEAD_WIDTH] = hw;
	PSL_plotsymbol (GMT->PSL, x1, y1, dim, PSL_VECTOR);

	/* second, compressional arrow in opposite direction */

	psvelo_trace_arrow (GMT, slon, slat, -dx, -dy, sscale, &x1, &y1, &x2, &y2);

	if (eps2 > 0.0) {
		gmt_M_double_swap (x1, x2);
		gmt_M_double_swap (y1, y2);
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

	dim[PSL_VEC_XTIP] = x2;
	dim[PSL_VEC_YTIP] = y2;
	dim[PSL_VEC_TAIL_WIDTH] = vw;
	dim[PSL_VEC_HEAD_LENGTH] = hl;
	dim[PSL_VEC_HEAD_WIDTH] = hw;
	PSL_plotsymbol (GMT->PSL, x1, y1, dim, PSL_VECTOR);

	return 0;
}

GMT_LOCAL int psvelo_trace_wedge (double spin, double sscale, double wedge_amp, int lines, double *x, double *y) {
	/* make a rotation rate wedge and return in x,y */

	/* Kurt Feigl, from code by D. Dong */

	/*   INPUT VARIABLES: */
	/*   slat        - latitude, in degrees of arrow tail */
	/*   slon        - longitude in degrees of arrow tail */
	/*   sscale      : scaling factor for size (radius) of wedge */
	/*   wedge_amp   : scaling factor for angular size of wedge */
	/*   spin        : CW spin rate in rad/yr */
	/*   line        : if true, draw lines                  */

	int nstep, i1, i, nump;
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

GMT_LOCAL int psvelo_trace_sigwedge (double spin, double spinsig, double sscale, double wedge_amp, double *x, double *y) {
	/* make a rotation rate uncertainty wedge and return in x,y */

	/* Kurt Feigl, from code by D. Dong */

	/*   INPUT VARIABLES: */
	/*   slat        - latitude, in degrees of arrow tail */
	/*   slon        - longitude in degrees of arrow tail */
	/*   sscale      : scaling factor for size (radius) of wedge */
	/*   wedge_amp   : scaling factor for angular size of wedge */
	/*   spin,spinsig:CW rotation rate and sigma in rad/yr */

	int nstep, i, nump;
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

GMT_LOCAL void psvelo_paint_wedge (struct PSL_CTRL *PSL, double x0, double y0, double spin, double spinsig, double sscale, double wedge_amp, double t11, double t12, double t21, double t22, int polygon, double *rgb, int epolygon, double *ergb, int outline) {

	/* Make a wedge at center x0,y0  */

#define NPOINTS 1000

	int npoints = NPOINTS, i;

	/* relative to center of ellipse */
	double dxe[NPOINTS], dye[NPOINTS];
	/* absolute paper coordinates */
	double axe[NPOINTS], aye[NPOINTS];
	gmt_M_unused(outline);

	/* draw wedge */

	npoints = psvelo_trace_wedge (spin, 1.0, wedge_amp, true, dxe, dye);

	for (i = 0; i <= npoints - 1; i++)
		psvelo_transform_local (x0, y0, dxe[i], dye[i], sscale, t11, t12, t21, t22, &axe[i], &aye[i]);

	if (polygon) {
		PSL_setfill (PSL, rgb, 1);
		PSL_plotpolygon (PSL, axe, aye, npoints);
	}
	else
		PSL_plotline (PSL, axe, aye, npoints, PSL_MOVE|PSL_STROKE|PSL_CLOSE);

	/* draw uncertainty wedge */

	npoints = psvelo_trace_sigwedge (spin, spinsig, 1.0,wedge_amp, dxe, dye);

	for (i = 0; i < npoints - 1; i++) psvelo_transform_local (x0, y0, dxe[i], dye[i], sscale, t11, t12, t21, t22, &axe[i], &aye[i]);

	if (epolygon) {
		PSL_setfill (PSL, ergb, 1);
		PSL_plotpolygon (PSL, axe, aye, npoints - 1);
	}
	else
		PSL_plotline (PSL, axe, aye, npoints - 1, PSL_MOVE|PSL_STROKE|PSL_CLOSE);
}

/* end of utilvelo.c */

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSVELO_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSVELO_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->A.S.size_x = VECTOR_HEAD_LENGTH * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 9p */
	C->A.S.v.h_length = (float)C->A.S.size_x;	/* 9p */
	C->A.S.v.v_angle = 30.0f;
	C->A.S.v.status = PSL_VEC_END + PSL_VEC_FILL + PSL_VEC_OUTLINE;
	C->A.S.v.pen = GMT->current.setting.map_default_pen;
	if (gmt_M_compat_check (GMT, 4)) GMT->current.setting.map_vector_shape = 0.4;	/* Historical reasons */
	C->A.S.v.v_shape = (float)GMT->current.setting.map_vector_shape;
	C->D.scale = 1.0;
	gmt_init_fill (GMT, &C->E.fill, 1.0, 1.0, 1.0);
	gmt_init_fill (GMT, &C->G.fill, 0.0, 0.0, 0.0);
	C->S.wedge_amp = 1.e7;
	C->S.conrad = 1.0;
	C->S.font = GMT->current.setting.font_annot[GMT_PRIMARY];
	C->S.font.size = 9;
	C->W.pen = GMT->current.setting.map_default_pen;
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct PSVELO_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the psvelo synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] %s %s -S<symbol>[<scale>][</args>][+f<font>] [-A<vecpar>] [%s] [-C<cpt>] [-D<scale>] [-E<fill>] "
		"[-G<fill>] [-H[<scale>]] [-I[<intens>]] %s[-L[<pen>][+c[f|l]]] [-N] %s%s[%s] [%s] "
		"[-W[<pen>][+c[f|l]]] [%s] [%s] [-Z[m|e|n|u][+e]] %s[%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_J_OPT, GMT_Rgeo_OPT, GMT_B_OPT, API->K_OPT, API->O_OPT, API->P_OPT, GMT_U_OPT, GMT_V_OPT, GMT_X_OPT,
		GMT_Y_OPT, API->c_OPT, GMT_di_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT, GMT_p_OPT, GMT_qi_OPT, GMT_tv_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Option (API, "<");
	GMT_Option (API, "J-,R");
	GMT_Usage (API, 1, "\n-S<symbol>[<scale>][</args>][+f<font>]");
	GMT_Usage (API, -2, "Select symbol type and <scale> (plus optional font; see documentation). "
		"If <scale> is not given then it is read from the first column after the required columns. "
		"Choose from the following geodetic symbols:");
	GMT_Usage (API, 3, "e: Velocity ellipses: in X,Y,Vx,Vy,SigX,SigY,CorXY,name format. "
		"Append <confidence> value (0-1) for error ellipse or give 0 to not draw the ellipse.");
	GMT_Usage (API, 3, "r: Velocity ellipses: in X,Y,Vx,Vy,a,b,theta,name format. "
		"Append <confidence> value (0-1) for error ellipse or give 0 to not draw the ellipse.");
	GMT_Usage (API, 3, "n: Anisotropy: in X,Y,Vx,Vy.");
	GMT_Usage (API, 3, "w: Rotational wedges: in X,Y,Spin,Spinsig. "
		"Append <wedgemag> value to scale the the Spin values [1].");
	GMT_Usage (API, 3, "x: Strain crosses: in X,Y,Eps1,Eps2,Theta.n");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Option (API, "B-");
	GMT_Usage (API, 1, "\n-A<vecpar>");
	GMT_Usage (API, -2, "Specify arrow head attributes:");
	gmt_vector_syntax (API->GMT, 15, 3);
	GMT_Usage (API, -2, "Default is %gp+gblack+p1p", VECTOR_HEAD_LENGTH);
	GMT_Usage (API, 1, "\n-C<cpt>");
	GMT_Usage (API, -2, "Use CPT to assign colors based on vector magnitude. "
		"For other coloring options, see -W and -Z.");
	GMT_Usage (API, 1, "\n-D<scale>");
	GMT_Usage (API, -2, "Multiply uncertainties by <scale> (for -Se and -Sw options only).");
	GMT_Usage (API, 1, "\n-E<fill>");
	GMT_Usage (API, -2, "Set color used for uncertainty ellipses and wedges [no fill]; see -G for syntax. "
		"For other coloring options, see -L and -Z.");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Specify color or pattern for symbol fill [no fill].");
	GMT_Usage (API, 1, "\n-H[<scale>]");
	GMT_Usage (API, -2, "Scale symbol sizes (set via -S or input column) and pen attributes by factors read from scale column. "
		"he scale column follows the symbol size column.  Alternatively, append a fixed <scale>.");
	GMT_Usage (API, 1, "\n-I[<intens>]");
	GMT_Usage (API, -2, "Use the intensity to modulate the symbol fill color (requires -C or -G). "
		"If no intensity is given we expect it to follow the required columns in the data record.");
	GMT_Option (API, "K");
	GMT_Usage (API, 1, "\n-L[<pen>][+c[f|l]]");
	GMT_Usage (API, -2, "Draw line or symbol outline using the current pen (see -W). "
		"Optionally, append separate pen for error outlines [Same as -W].");
	GMT_Usage (API, 1, "\n-N Do Not skip/clip symbols that fall outside map border [Default will ignore those outside].");
	GMT_Option (API, "O,P");
	GMT_Option (API, "U,V");
	GMT_Usage (API, 1, "\n-W[<pen>][+c[f|l]]");
	GMT_Usage (API, -2, "Set pen attributes [%s].", gmt_putpen (API->GMT, &API->GMT->current.setting.map_default_pen));
	GMT_Option (API, "X");
	GMT_Usage (API, 1, "\n-Z[m|e|n|u][+e]");
	GMT_Usage (API, -2, "Select quantity to use with -C to look-up colors.  Choose among:");
	GMT_Usage (API, 3, "m: Magnitude of vector or rotation [Default].");
	GMT_Usage (API, 3, "e: East velocity component.");
	GMT_Usage (API, 3, "n: North velocity component.");
	GMT_Usage (API, 3, "u: User column (given after required columns).");
	GMT_Usage (API, -2, "Note: Color selected will replace -G<fill>.  Append +e to instead act as -E<fill>.");
	GMT_Option (API, "c,di,e,h,i,p,qi,T,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct PSVELO_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to psvelo and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_set;
	int n;
	bool got_A = false, got_shape = false;
	char txt[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, symbol, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	symbol = (gmt_M_is_geographic (GMT, GMT_IN)) ? '=' : 'v';	/* Type of vector */

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Change size of arrow head */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				got_A = true;
				if (gmt_M_compat_check (GMT, 4) && (strchr (opt->arg, '/') && !strchr (opt->arg, '+'))) {	/* Old-style args */
					sscanf (opt->arg, "%[^/]/%[^/]/%s", txt, txt_b, txt_c);
					Ctrl->A.S.v.v_width = (float)gmt_M_to_inch (GMT, txt);
					Ctrl->A.S.v.h_length = (float)gmt_M_to_inch (GMT, txt_b);
					Ctrl->A.S.v.h_width = (float)gmt_M_to_inch (GMT, txt_c);
					Ctrl->A.S.v.v_angle = (float)atand (0.5 * Ctrl->A.S.v.h_width / Ctrl->A.S.v.h_length);
					Ctrl->A.S.v.status |= PSL_VEC_OUTLINE2;
					Ctrl->A.S.symbol = GMT_SYMBOL_VECTOR_V4;
				}
				else {
					if (opt->arg[0] == '+') {	/* No size (use default), just attributes */
						n_errors += gmt_parse_vector (GMT, symbol, opt->arg, &Ctrl->A.S);
					}
					else {	/* Size, plus possible attributes */
						n = sscanf (opt->arg, "%[^+]%s", txt, txt_b);	/* txt_a should be symbols size with any +<modifiers> in txt_b */
						if (n == 1) txt_b[0] = 0;	/* No modifiers present, set txt_b to empty */
						Ctrl->A.S.size_x = gmt_M_to_inch (GMT, txt);	/* Length of vector */
						n_errors += gmt_parse_vector (GMT, symbol, txt_b, &Ctrl->A.S);
					}
					Ctrl->A.S.symbol = PSL_VECTOR;
					if (strstr (opt->arg, "+h")) got_shape = true;	/* User specified vector head shape */
				}
				break;
			case 'C':	/* Select CPT for coloring */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.active = true;
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':	/* Rescale sigmas */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				Ctrl->D.active = true;
				Ctrl->D.scale = atof (opt->arg);
				break;
			case 'E':	/* Set color for error ellipse  */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				if (gmt_getfill (GMT, opt->arg, &Ctrl->E.fill)) {
					gmt_fill_syntax (GMT, 'E', NULL, " ");
					n_errors++;
				}
				Ctrl->E.active = true;
				break;
			case 'G':	/* Set Gray shade for polygon */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				Ctrl->G.active = true;
				if (gmt_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					gmt_fill_syntax (GMT, 'G', NULL, " ");
					n_errors++;
				}
				break;
			case 'H':		/* Overall symbol/pen scale column provided */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->H.active);
				Ctrl->H.active = true;
				if (opt->arg[0]) {	/* Gave a fixed scale - no reading from file */
					Ctrl->H.value = atof (opt->arg);
					Ctrl->H.mode = PSVELO_CONST_SCALE;
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
			case 'L':	/* Draw the outline */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				Ctrl->L.active = true;
				if (opt->arg[0]) {
					Ctrl->L.error_pen = true;
					if (gmt_getpen (GMT, opt->arg, &Ctrl->L.pen)) {
						gmt_pen_syntax (GMT, 'L', NULL, " ", NULL, 0);
						n_errors++;
					}
				}
				break;
			case 'N':	/* Do not skip points outside border */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				Ctrl->N.active = true;
				break;
			case 'S':	/* Get symbol [and size] */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
 				txt_b[0] = '\0';
				if ((c = strstr (opt->arg, "+f"))) {	/* Gave font directly so handle that first */
 					if (c[2] == '0')
 						Ctrl->S.font.size = 0;
					else
						n_errors += gmt_getfont (GMT, &c[2], &(Ctrl->S.font));
					c[0] = '\0';	/* Temporarily chop off the font specification */
				}
 				if (strchr ("er", opt->arg[0])) {	/* Error ellipse with vector symbol for -Se and -Sr */
					strncpy (txt, &opt->arg[1], GMT_LEN256);	/* Copy of the args after -Se|r */
					n = 0;
					if (strchr (txt, '/')) {	/* We clearly have scale/confidence and possibly /fontsize (deprecated) */
						while (txt[n] && txt[n] != '/') n++; txt[n++] = 0;	/* Hide the /confidence part */
						Ctrl->S.scale = gmt_M_to_inch (GMT, txt);	/* Get symbol size */
					}
					sscanf (&txt[n], "%lf/%s", &Ctrl->S.confidence, txt_b);
					/* confidence scaling */
					Ctrl->S.conrad = sqrt (-2.0 * log (1.0 - Ctrl->S.confidence));
					/* Check for deprecated font syntax */
					if (txt_b[0]) Ctrl->S.font.size = gmt_convert_units (GMT, txt_b, GMT_PT, GMT_PT);
				}
				else if (strchr ("nx", opt->arg[0])) {	/* Simple one-parameter argument for -Sn and -Sx */
					if (opt->arg[1]) Ctrl->S.scale = gmt_M_to_inch (GMT, &opt->arg[1]);
				}
				else if (opt->arg[0] == 'w') {	/* Rotational wedge */
					strncpy (txt, &opt->arg[1], GMT_LEN256);
					n = 0;
					if (strchr (txt, '/')) {	/* We clearly have scale/wedgemag  */
						while (txt[n] && txt[n] != '/') n++; txt[n++] = 0;	/* Hide the /wedgemag part */
						Ctrl->S.scale = gmt_M_to_inch (GMT, txt);	/* Get symbol size */
					}
					Ctrl->S.wedge_amp = (txt[n]) ? atof (&txt[n]) : 1.0;
				}
				switch (opt->arg[0]) {	/* Set modes and expected input columns */
					case 'e':
						Ctrl->S.symbol = CINE;	Ctrl->S.n_cols = 7;
						Ctrl->S.readmode = READ_ELLIPSE;
						break;
					case 'r':
						Ctrl->S.symbol = CINE;	Ctrl->S.n_cols = 7;
						Ctrl->S.readmode = READ_ROTELLIPSE;
						break;
					case 'n':
						Ctrl->S.symbol = ANISO;	Ctrl->S.n_cols = 4;
						Ctrl->S.readmode = READ_ANISOTROPY;
						break;
					case 'w':
						Ctrl->S.symbol = WEDGE;	Ctrl->S.n_cols = 4;
						Ctrl->S.readmode = READ_WEDGE;
						break;
					case 'x':
						Ctrl->S.symbol = CROSS;	Ctrl->S.n_cols = 5;
						Ctrl->S.readmode = READ_CROSS;
						break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -S: Unrecognized symbol code %s\n", opt->arg);
						n_errors++;
						break;
				}
				if (c) c[0] = '+';	/* Restore font specification */
				if (gmt_M_is_zero (Ctrl->S.scale)) Ctrl->S.read = true;	/* Must get size from input file */
				break;
			case 'W':	/* Set line attributes */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				Ctrl->W.active = true;
				if (opt->arg[0] && gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', NULL, " ", NULL, 0);
					n_errors++;
				}
				break;
			case 'Z':	/* Set items to control CPT coloring */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				Ctrl->Z.active = true;
				if (opt->arg[0] && (c = strstr (opt->arg, "+e"))) {	/* Paint error part of symbol instead (-E) */
					Ctrl->Z.item = PSVELO_E_FILL;
					c[0] = '\0';	/* Temporarily chop off the modifier */
				}
				switch (opt->arg[0]) {
					case 'm':	case '\0': Ctrl->Z.mode = PSVELO_V_MAG;	break;
					case 'e':	Ctrl->Z.mode = PSVELO_V_EAST;	break;
					case 'n':	Ctrl->Z.mode = PSVELO_V_NORTH;	break;
					case 'r':	Ctrl->Z.mode = PSVELO_R_MAG;	break;
					case 'u':	Ctrl->Z.mode = PSVELO_V_USER;	break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -Z: Unrecognized mode %s\n", opt->arg);
						n_errors++;
						break;
				}
				if (c) c[0] = '+';	/* Restore modifier */
				break;

			/* Illegal options */

		}
	}

	if (Ctrl->S.symbol == CROSS && !got_shape) Ctrl->A.S.v.v_shape = 0.1;	/* Traditional default cross vector shape if none given */
	gmt_consider_current_cpt (API, &Ctrl->C.active, &(Ctrl->C.file));

        /* Only one allowed */
	n_set = (Ctrl->S.readmode == READ_ELLIPSE) + (Ctrl->S.readmode == READ_ROTELLIPSE) + (Ctrl->S.readmode == READ_ANISOTROPY) + (Ctrl->S.readmode == READ_CROSS) + (Ctrl->S.readmode == READ_WEDGE);
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, n_set > 1, "Only one -S setting is allowed.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && ! (Ctrl->S.readmode == READ_ELLIPSE || Ctrl->S.readmode == READ_WEDGE), "Option -D requires -Se|w.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && !Ctrl->C.active, "Option -Z requires -C.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->Z.item == PSVELO_E_FILL && Ctrl->E.active, "Options -C -Z+e cannot be combined with -E.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->Z.item == PSVELO_G_FILL && Ctrl->G.active, "Options -C -Z cannot be combined with -G.\n");

	if (!got_A && Ctrl->W.active) Ctrl->A.S.v.pen = Ctrl->W.pen;	/* Set vector pen to that given by -W if -A was not set */
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LOCAL void psvelo_set_colorfill (struct GMT_CTRL *GMT, struct PSVELO_CTRL *Ctrl, struct GMT_PALETTE *P, double value, double ival) {
	/* Called if -C was given.  Selects and updates color fills and possibly pen colors */
	struct GMT_FILL *F = (Ctrl->Z.item == PSVELO_G_FILL) ? &Ctrl->G.fill : &Ctrl->E.fill;
	if (P) gmt_get_fill_from_z (GMT, P, value, F);	/* Update color based on value and CPT */
	if (Ctrl->I.active) {	/* Modulate color based on intensity ival */
		gmt_illuminate (GMT, ival, F->rgb);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Illumination value used is %h\n", ival);
	}
	if (Ctrl->L.pen.cptmode & 1) {	/* Also change error pen color via CPT */
		gmt_M_rgb_copy (Ctrl->L.pen.rgb, F->rgb);
		gmt_setpen (GMT, &Ctrl->L.pen);
	}
	if (Ctrl->W.pen.cptmode & 1) {	/* Also change pen color via CPT */
		gmt_M_rgb_copy (Ctrl->W.pen.rgb, F->rgb);
		gmt_setpen (GMT, &Ctrl->W.pen);
		if (!Ctrl->L.error_pen)	/* No -L pen so duplicate the change */
			gmt_M_rgb_copy (Ctrl->L.pen.rgb, Ctrl->W.pen.rgb);
	}
}

EXTERN_MSC int GMT_psvelo (void *V_API, int mode, void *args) {
	int ix = 0, iy = 1, n_rec = 0, justify;
	int plot_ellipse = true, plot_vector = true, error = false;
	unsigned int xcol = 0, tcol_f = 0, tcol_s = 0, scol = 0, icol = 0, n_warn = 0;
	bool set_g_fill, set_e_fill;

	double plot_x, plot_y, vxy[2], plot_vx, plot_vy, length, s, dim[PSL_MAX_DIMS];
	double eps1 = 0.0, eps2 = 0.0, spin = 0.0, spinsig = 0.0, theta = 0.0, *in = NULL;
	double direction = 0, small_axis = 0, great_axis = 0, sigma_x, sigma_y, corr_xy;
	double t11 = 1.0, t12 = 0.0, t21 = 0.0, t22 = 1.0, hl, hw, vw, ssize, headpen_width = 0.0;
	double z_val, e_val, value, scale, size, i_value, nominal_size;

	char *station_name = NULL;

	struct GMT_RECORD *In = NULL;
	struct GMT_PALETTE *CPT = NULL;
	struct GMT_PEN current_pen;
	struct PSVELO_CTRL *Ctrl = NULL;
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

	/*---------------------------- This is the psvelo main code ----------------------------*/

	set_e_fill = Ctrl->E.active;	set_g_fill = Ctrl->G.active;
	if (Ctrl->C.active) {
		if ((CPT = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (Ctrl->Z.item == PSVELO_G_FILL) set_g_fill = true;	/* Since we will set it via CPT lookup */
		if (Ctrl->Z.item == PSVELO_E_FILL) set_e_fill = true;	/* Since we will set it via CPT lookup */
	}
	else if (Ctrl->I.active && Ctrl->I.mode == 0) {	/* Constant illumination with constant intensity can be done once before data loop */
		gmt_illuminate (GMT, Ctrl->I.value, Ctrl->E.fill.rgb);
		gmt_illuminate (GMT, Ctrl->I.value, Ctrl->G.fill.rgb);
		Ctrl->I.active = false;	/* So we don't do this again */
	}
	i_value = Ctrl->I.value;	/* May be replaced in the loop if variable intensity was given */
	if (Ctrl->S.symbol == CINE && Ctrl->S.confidence > 0.0 && !(Ctrl->E.active || Ctrl->L.active))
		Ctrl->L.active = true;	/* If confidence is > 0 but neither -E or -L is set then we turn on -L to pick up -W pen (below) */
	if (Ctrl->L.active && !Ctrl->L.error_pen)	/* Duplicate -W to -L */
		gmt_M_memcpy (&Ctrl->L.pen, &Ctrl->W.pen, 1, struct GMT_PEN);

	if (gmt_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_PROJECTION_ERROR);

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
	gmt_set_basemap_orders (GMT, Ctrl->N.active ? GMT_BASEMAP_FRAME_BEFORE : GMT_BASEMAP_FRAME_AFTER, GMT_BASEMAP_GRID_BEFORE, GMT_BASEMAP_ANNOT_AFTER);
	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */
	gmt_map_basemap (GMT);	/* Basemap before data */

	gmt_M_memset (&current_pen, 1, struct GMT_PEN);
	gmt_M_memset (dim, PSL_MAX_DIMS, double);
	gmt_setpen (GMT, &Ctrl->W.pen);
	PSL_setfont (PSL, GMT->current.setting.font_annot[GMT_PRIMARY].id);

	if (!Ctrl->N.active) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);
	gmt_init_vector_param (GMT, &Ctrl->A.S, true, Ctrl->W.active, &Ctrl->W.pen, Ctrl->G.active, &Ctrl->G.fill);
	if (Ctrl->A.S.symbol == PSL_VECTOR) Ctrl->A.S.v.v_width = (float)(Ctrl->W.pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);

	ix = (GMT->current.setting.io_lonlat_toggle[0]);	iy = 1 - ix;	/* Deal with -: */

	  /* 1. Add user column for coloring, if requested */
	if (Ctrl->Z.mode == PSVELO_V_USER) Ctrl->S.n_cols++;	/* Need to read one extra column */
	/* 2. Add scale from file, if missing */
	if (Ctrl->S.read) {	/* Read symbol size from file */
		scol = Ctrl->S.n_cols;	/* Column ID with scales */
		Ctrl->S.n_cols++;	/* Must read an extra column */
		gmt_set_column_type (GMT, GMT_IN, scol, GMT_IS_DIMENSION);
	}
	else	/* Fixed symbol scale */
		nominal_size = scale = Ctrl->S.scale;
	/* 3. Add scaling from file, if requested */
	if (Ctrl->H.active && Ctrl->H.mode == PSVELO_READ_SCALE) {
		xcol = Ctrl->S.n_cols;
		Ctrl->S.n_cols++;	/* Read scaling from data file */
		gmt_set_column_type (GMT, GMT_IN, xcol, GMT_IS_FLOAT);
	}
	/* 4. Add intensity from file, if requested */
	if (Ctrl->I.mode) {	/* Read intensity from data file */
		icol = Ctrl->S.n_cols;	/* Column id for intensity */
		Ctrl->S.n_cols++;	/* One more data column required */
		gmt_set_column_type (GMT, GMT_IN, icol, GMT_IS_FLOAT);
	}
	/* 5. Add transparencies from file, if requested */
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

	GMT_Set_Columns (API, GMT_IN, Ctrl->S.n_cols, GMT_COL_FIX);

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	if (Ctrl->S.readmode == READ_ELLIPSE || Ctrl->S.readmode == READ_ROTELLIPSE) GMT_Report (API, GMT_MSG_INFORMATION, "psvelo: 2-D confidence interval and scaling factor %f %f\n", Ctrl->S.confidence, Ctrl->S.conrad);

	if (Ctrl->D.active)  GMT_Report (API, GMT_MSG_INFORMATION, "Rescaling uncertainties by a factor of %f\n", Ctrl->D.scale);

	if (Ctrl->S.symbol == CINE || Ctrl->S.symbol == CROSS) {
		if (Ctrl->A.S.v.status & PSL_VEC_OUTLINE2) {	/* Vector head outline pen specified separately */
			PSL_defpen (PSL, "PSL_vecheadpen", Ctrl->A.S.v.pen.width, Ctrl->A.S.v.pen.style, Ctrl->A.S.v.pen.offset, Ctrl->A.S.v.pen.rgb);
			headpen_width = 0.5 * Ctrl->A.S.v.pen.width;
		}
		else {	/* Reset to default pen */
			if (Ctrl->W.active) {	/* Vector head outline pen default is half that of stem pen */
				PSL_defpen (PSL, "PSL_vecheadpen", Ctrl->W.pen.width, Ctrl->W.pen.style, Ctrl->W.pen.offset, Ctrl->W.pen.rgb);
				headpen_width = 0.5 * Ctrl->W.pen.width;
			}
		}
	}
	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_MIXED, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (gmt_M_rec_is_any_header (GMT)) 	/* Skip all table and segment headers */
				continue;
			if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			assert (In->text != NULL);						/* Should never get here */
		}
		if (In->data == NULL) {
			gmt_quit_bad_record (API, In);
			Return (API->error);
		}

		if (gmt_M_is_dnan (In->data[GMT_X]) || gmt_M_is_dnan (In->data[GMT_Y]))	/* Probably a non-recognized header since we got NaNs */
			continue;

		in = In->data;
		station_name = In->text;

		/* Data record to process */

		n_rec++;

		if (Ctrl->S.readmode == READ_ELLIPSE) {
			vxy[ix] = in[2];	vxy[iy] = in[3];
			sigma_x = in[4];	sigma_y = in[5];
			corr_xy = in[6];
			if (Ctrl->C.active) {	/* Compute/select the value parameters */
				switch (Ctrl->Z.mode) {
					case PSVELO_V_MAG:   z_val = hypot (vxy[0], vxy[1]);	e_val = hypot (sigma_x, sigma_y); break;
					case PSVELO_V_EAST:  z_val = vxy[0]; e_val = sigma_x;	break;
					case PSVELO_V_NORTH: z_val = vxy[1]; e_val = sigma_y;	break;
					case PSVELO_V_USER:  z_val = e_val = in[Ctrl->S.n_cols-1];	break;
				}
			}
			/* rescale uncertainties if necessary */
			if (Ctrl->D.active) {
				sigma_x = Ctrl->D.scale * sigma_x;
				sigma_y = Ctrl->D.scale * sigma_y;
			}
			if (fabs (sigma_x) < EPSIL && fabs (sigma_y) < EPSIL)
				plot_ellipse = false;
			else {
				plot_ellipse = true;
				psvelo_ellipse_convert (sigma_x, sigma_y, corr_xy, Ctrl->S.conrad, &small_axis, &great_axis, &direction);

				/* convert to degrees */
				direction = direction * R2D;
			}
		}
		else if (Ctrl->S.readmode == READ_ROTELLIPSE) {
			vxy[ix] = in[2];	vxy[iy] = in[3];
			great_axis = Ctrl->S.conrad*in[4];
			small_axis = Ctrl->S.conrad*in[5];
			direction = in[6];
			if (Ctrl->C.active) {	/* Compute/select the value parameters */
				switch (Ctrl->Z.mode) {
					case PSVELO_V_MAG:   z_val = hypot (vxy[0], vxy[1]);	e_val = hypot (great_axis, small_axis); break;
					case PSVELO_V_EAST:  z_val = vxy[0]; e_val = great_axis;	break;
					case PSVELO_V_NORTH: z_val = vxy[1]; e_val = small_axis;	break;
					case PSVELO_V_USER:  z_val = e_val = in[Ctrl->S.n_cols-1];	break;
				}
			}
			if (fabs (great_axis) < EPSIL && fabs (small_axis) < EPSIL)
				plot_ellipse = false;
			else
				plot_ellipse = true;
		}
		else if (Ctrl->S.readmode == READ_ANISOTROPY) {
			vxy[ix] = in[2];
			vxy[iy] = in[3];
		}
		else if (Ctrl->S.readmode == READ_CROSS) {
			eps1  = in[2];
			eps2  = in[3];
			theta = in[4];
		}
		else if (Ctrl->S.readmode == READ_WEDGE) {
			spin    = in[2];
			spinsig = in[3];
			if (Ctrl->C.active) {
				switch (Ctrl->Z.mode) {
					case PSVELO_V_MAG:  z_val = spin;	e_val = spinsig; break;
					case PSVELO_V_USER: z_val = e_val = in[Ctrl->S.n_cols-1];	break;
				}
			}
			if (Ctrl->D.active) spinsig = spinsig * Ctrl->D.scale;
		}
		if (Ctrl->S.read) nominal_size = scale = in[scol];

		if (!Ctrl->N.active) {
			gmt_map_outside (GMT, in[GMT_X], in[GMT_Y]);
			if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) continue;
		}

		gmt_geo_to_xy (GMT, in[GMT_X], in[GMT_Y], &plot_x, &plot_y);

		value = (Ctrl->Z.item == PSVELO_E_FILL) ? e_val : z_val;	/* Select which value for color lookup - if active */
		if (Ctrl->I.mode) i_value = in[icol];
		/* Possibly update E or G fills based on value and/ore intensities, then set updated colors in PS */
		psvelo_set_colorfill (GMT, Ctrl, CPT, value, i_value);
		gmt_init_vector_param (GMT, &Ctrl->A.S, true, Ctrl->W.active, &Ctrl->W.pen, Ctrl->G.active, &Ctrl->G.fill);

		if (GMT->common.t.variable) {	/* Update the transparency for current symbol (or -t was given) */
			double transp[2] = {0.0, 0.0};
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
		size = scale;
		if (Ctrl->H.active) {	/* Variable scaling of symbol size and pen width */
			double scl = (Ctrl->H.mode == PSVELO_READ_SCALE) ? in[xcol] : Ctrl->H.value;
			size *= scl;
		}

		switch (Ctrl->S.symbol) {
			case CINE:
				plot_vector = (hypot (vxy[0], vxy[1]) < 1.e-8) ? false : true;
				psvelo_trace_arrow (GMT, in[GMT_X], in[GMT_Y], vxy[0], vxy[1], size, &plot_x, &plot_y, &plot_vx, &plot_vy);
				psvelo_get_trans (GMT, in[GMT_X], in[GMT_Y], &t11, &t12, &t21, &t22);
				if (plot_ellipse) {	/* Optionally fill [-E] and optionally outline [-L] the error ellipse */
					if (Ctrl->L.active) {	/* Draw ellipse outline */
						current_pen = Ctrl->L.pen;
						if (Ctrl->H.active) {
							double scl = (Ctrl->H.mode == PSVELO_READ_SCALE) ? in[xcol] : Ctrl->H.value;
							gmt_scale_pen (GMT, &current_pen, scl);
						}
						gmt_setpen (GMT, &current_pen);
					}
					/* Draw the ellipse */
					psvelo_paint_ellipse (GMT, plot_vx, plot_vy, direction, great_axis, small_axis, size,
						t11, t12, t21, t22, Ctrl->E.active, &Ctrl->E.fill, Ctrl->L.active);
				}
				if (plot_vector) {	/* Verify that vector length is not ridiculously small */
					length = hypot (plot_x-plot_vx, plot_y-plot_vy);	/* Length of arrow */
					if (length < Ctrl->A.S.v.h_length && Ctrl->A.S.v.v_norm < 0.0) {	/* No shrink requested yet head length exceeds total vector length */
						GMT_Report (API, GMT_MSG_INFORMATION, "Vector head length exceeds overall vector length near line %d. Consider adding +n<norm> to -A\n", n_rec);
						n_warn++;
					}
					s = (length < Ctrl->A.S.v.v_norm) ? length / Ctrl->A.S.v.v_norm : 1.0;
					if (s < Ctrl->A.S.v.v_norm_limit) s = Ctrl->A.S.v.v_norm_limit;
					hw = s * Ctrl->A.S.v.h_width;
					hl = s * Ctrl->A.S.v.h_length;
					vw = s * Ctrl->A.S.v.v_width;
					if (vw < (2.0/PSL_DOTS_PER_INCH)) vw = 2.0/PSL_DOTS_PER_INCH;	/* Minimum width set */
					if (Ctrl->A.S.v.status & PSL_VEC_OUTLINE2) {
						current_pen = Ctrl->A.S.v.pen;
						if (Ctrl->H.active) {
							double scl = (Ctrl->H.mode == PSVELO_READ_SCALE) ? in[xcol] : Ctrl->H.value;
							gmt_scale_pen (GMT, &current_pen, scl);
						}
						gmt_setpen (GMT, &current_pen);
					}
					dim[PSL_VEC_XTIP]        = plot_vx;
					dim[PSL_VEC_YTIP]        = plot_vy;
					dim[PSL_VEC_TAIL_WIDTH]  = vw;
					dim[PSL_VEC_HEAD_LENGTH] = hl;
					dim[PSL_VEC_HEAD_WIDTH]  = hw;
					dim[PSL_VEC_HEAD_SHAPE]  = Ctrl->A.S.v.v_shape;
					if (Ctrl->W.active) {
						current_pen = Ctrl->W.pen;
						if (Ctrl->H.active) {
							double scl = (Ctrl->H.mode == PSVELO_READ_SCALE) ? in[xcol] : Ctrl->H.value;
							gmt_scale_pen (GMT, &current_pen, scl);
						}
						gmt_setpen (GMT, &current_pen);
					}
					if (Ctrl->A.S.symbol == GMT_SYMBOL_VECTOR_V4) {	/* Old GMT4 vector selected */
						double *this_rgb = (set_g_fill) ? Ctrl->G.fill.rgb : GMT->session.no_rgb;
						psl_vector_v4 (PSL, plot_x, plot_y, dim, this_rgb, Ctrl->L.active);
					}
					else {
						dim[PSL_VEC_STATUS]          = (double)Ctrl->A.S.v.status;
						dim[PSL_VEC_HEAD_TYPE_BEGIN] = (double)Ctrl->A.S.v.v_kind[0];
						dim[PSL_VEC_HEAD_TYPE_END]   = (double)Ctrl->A.S.v.v_kind[1];
						dim[PSL_VEC_HEAD_PENWIDTH]   = (headpen_width > 0.0) ? headpen_width : 0.5 * Ctrl->W.pen.width;
						if (Ctrl->A.S.v.status & PSL_VEC_FILL2)
							gmt_setfill (GMT, &Ctrl->A.S.v.fill, Ctrl->W.active);
						else if (set_g_fill)
							gmt_setfill (GMT, &Ctrl->G.fill, Ctrl->W.active);
						PSL_plotsymbol (PSL, plot_x, plot_y, dim, PSL_VECTOR);
					}
					justify = ((plot_vx - plot_x) > 0.0) ? PSL_MR : PSL_ML;
					if (Ctrl->S.font.size > 0.0 && station_name && station_name[0])	/* 1 inch = 2.54 cm */
						PSL_plottext (PSL, plot_x + (PSL_MC - justify) / 25.4 , plot_y, Ctrl->S.font.size, station_name, ANGLE, justify, FORM);
				}
				else {	/* vector too small, just place an circle there instead */
					if (set_g_fill)
						gmt_setfill (GMT, &Ctrl->G.fill, 1);
					ssize = GMT_DOT_SIZE;
					PSL_plotsymbol (PSL, plot_x, plot_y, &ssize, PSL_CIRCLE);
					justify = PSL_TC;
					if (Ctrl->S.font.size > 0.0 && station_name && station_name[0])	/* Place station name */
						PSL_plottext (PSL, plot_x, plot_y - 1.0 / 25.4, Ctrl->S.font.size, station_name, ANGLE, justify, FORM);
				}
				break;
			case ANISO:
				psvelo_trace_arrow (GMT, in[GMT_X], in[GMT_Y], vxy[0], vxy[1], size, &plot_x, &plot_y, &plot_vx, &plot_vy);
				current_pen = Ctrl->W.pen;
				if (Ctrl->H.active) {
					double scl = (Ctrl->H.mode == PSVELO_READ_SCALE) ? in[xcol] : Ctrl->H.value;
					gmt_scale_pen (GMT, &current_pen, scl);
				}
				gmt_setpen (GMT, &current_pen);
				PSL_plotsegment (PSL, plot_x, plot_y, plot_vx, plot_vy);
				break;
			case CROSS:
				/* triangular arrowheads */
				current_pen = Ctrl->W.pen;
				if (Ctrl->H.active) {
					double scl = (Ctrl->H.mode == PSVELO_READ_SCALE) ? in[xcol] : Ctrl->H.value;
					gmt_scale_pen (GMT, &current_pen, scl);
				}
				psvelo_trace_cross (GMT, in[GMT_X],in[GMT_Y],eps1,eps2,theta,size,Ctrl->A.S.v.v_width,Ctrl->A.S.v.h_length,
					Ctrl->A.S.v.h_width,Ctrl->A.S.v.v_shape,Ctrl->L.active,&(current_pen));
				break;
			case WEDGE:
				PSL_comment (PSL, "begin wedge number %li", n_rec);
				gmt_geo_to_xy (GMT, in[GMT_X], in[GMT_Y], &plot_x, &plot_y);
				psvelo_get_trans (GMT, in[GMT_X], in[GMT_Y], &t11, &t12, &t21, &t22);
				psvelo_paint_wedge (PSL, plot_x, plot_y, spin, spinsig, size, Ctrl->S.wedge_amp, t11, t12, t21, t22,
					set_g_fill, Ctrl->G.fill.rgb, set_e_fill, Ctrl->E.fill.rgb, Ctrl->L.active);
				break;
		}
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}

	if (GMT->common.t.variable) {	/* Reset the transparencies */
		double transp[2] = {0.0, 0.0};
		PSL_settransparencies (PSL, transp);
	}

	if (n_warn) GMT_Report (API, GMT_MSG_INFORMATION, "%d vector heads had length exceeding the vector length and were skipped. Consider the +n<norm> modifier to -A\n", n_warn);
	GMT_Report (API, GMT_MSG_INFORMATION, "Number of records read: %li\n", n_rec);

	if (!Ctrl->N.active) gmt_map_clip_off (GMT);

	PSL_setdash (PSL, NULL, 0);

	gmt_map_basemap (GMT);	/* Basemap after data */
	gmt_plane_perspective (GMT, -1, 0.0);
	gmt_plotend (GMT);

	Return (GMT_NOERROR);
}

EXTERN_MSC int GMT_velo (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_ERROR, "Shared GMT module not found: velo\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_psvelo (V_API, mode, args);
}
