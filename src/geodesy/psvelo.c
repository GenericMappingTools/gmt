/*--------------------------------------------------------------------
 *
 *    Copyright (c) 1996-2012 by G. Patau
 *    Copyright (c) 2013-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *    Donated to the GMT project by G. Patau upon her retirement from IGPG
 *    Distributed under the Lesser GNU Public Licence
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*

psvelo will read <x,y> pairs (or <lon,lat>) from inputfile and
plot symbols on a map. Velocity ellipses, strain
crosses, or strain wedges, may be specified, some of which require
additional columns of data.  Only one symbol may be plotted at a time.
PostScript code is written to stdout.


 Author:	Kurt Feigl
 Date:		7 July 1998
 Version:	5
 Roots:		based on psxy.c
 Adapted to version 3.3 by Genevieve Patau (25 June 1999)
 Last modified : 18 February 2000.  Ported to GMT 5 by P. Wessel

*/

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"psvelo"
#define THIS_MODULE_MODERN_NAME	"velo"
#define THIS_MODULE_LIB		"geodesy"
#define THIS_MODULE_PURPOSE	"Plot velocity vectors, crosses, and wedges"
#define THIS_MODULE_KEYS	"<D{,>X}"
#define THIS_MODULE_NEEDS	"Jd"
#define THIS_MODULE_OPTIONS "-:>BHJKOPRUVXYdehipqt" GMT_OPT("c")

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
	struct PSVELO_L {	/* -L */
		bool active;
	} L;
	struct PSVELO_N {	/* -N */
		bool active;
	} N;
	struct PSVELO_S {	/* -r<fill> */
		bool active;
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
};

/* COntent of old utilvelo.c is here */

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

	dim[0] = x2, dim[1] = y2;
	dim[2] = vw, dim[3] = hl, dim[4] = hw;
	dim[5] = vector_shape, dim[6] = PSL_VEC_END | PSL_VEC_FILL;
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

	dim[0] = x2, dim[1] = y2;
	dim[2] = vw, dim[3] = hl, dim[4] = hw;
	PSL_plotsymbol (GMT->PSL, x1, y1, dim, PSL_VECTOR);

	/* compression component */
	dx = eps2 * s;
	dy = eps2 * c;
	dim[6] = PSL_VEC_BEGIN | PSL_VEC_FILL;
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

	dim[0] = x2, dim[1] = y2;
	dim[2] = vw, dim[3] = hl, dim[4] = hw;
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

	dim[0] = x2, dim[1] = y2;
	dim[2] = vw, dim[3] = hl, dim[4] = hw;
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
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] %s %s [-A<vecpar>] [%s] [-D<sigscale>]\n", name, GMT_J_OPT, GMT_Rgeo_OPT, GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-G<fill>] %s[-L] [-N] %s%s[-S<symbol><args>[+f<font>]]\n", API->K_OPT, API->O_OPT, API->P_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-V] [-W<pen>] [%s]\n", GMT_U_OPT, GMT_X_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] %s[%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\n", GMT_Y_OPT, API->c_OPT, GMT_di_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT, GMT_p_OPT, GMT_qi_OPT, GMT_t_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Option (API, "J-,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<,B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Specify arrow head attributes:\n");
	gmt_vector_syntax (API->GMT, 15);
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is %gp+gblack+p1p\n", VECTOR_HEAD_LENGTH);
	GMT_Message (API, GMT_TIME_NONE, "\t-D Multiply uncertainties by <sigscale>. (Se and Sw only)i\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Set color used for uncertainty wedges in -Sw option.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify color (for symbols/polygons) or pattern (for polygons). fill can be either\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   1) <r/g/b> (each 0-255) for color or <gray> (0-255) for gray-shade [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   2) p[or P]<iconsize>/<pattern> for predefined patterns (0-90).\n");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Draw line or symbol outline using the current pen (see -W).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do Not skip/clip symbols that fall outside map border [Default will ignore those outside].\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Select symbol type and scale (plus optional font; see documentation). Choose between:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     e  Velocity ellipses: in X,Y,Vx,Vy,SigX,SigY,CorXY,name format.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     r  Velocity ellipses: in X,Y,Vx,Vy,a,b,theta,name format.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     n  Anisotropy : in X,Y,Vx,Vy.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     w  Rotational wedges: in X,Y,Spin,Spinsig.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     x  Strain crosses : in X,Y,Eps1,Eps2,Theta.\n");
	GMT_Option (API, "U,V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Set pen attributes [%s].\n", gmt_putpen (API->GMT, &API->GMT->current.setting.map_default_pen));
	GMT_Option (API, "X,c,di,e,h,i,p,qi,t,:,.");

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
	bool no_size_needed, got_A = false;
	char txt[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, symbol, *c = NULL;
	struct GMT_OPTION *opt = NULL;

	symbol = (gmt_M_is_geographic (GMT, GMT_IN)) ? '=' : 'v';	/* Type of vector */

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				if (GMT_Get_FilePath (GMT->parent, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Change size of arrow head */
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
				}
				break;
			case 'D':	/* Rescale Sigmas */
				Ctrl->D.active = true;
				sscanf (opt->arg, "%lf",&Ctrl->D.scale);
				break;
			case 'E':	/* Set color for error ellipse  */
				if (gmt_getfill (GMT, opt->arg, &Ctrl->E.fill)) {
					gmt_fill_syntax (GMT, 'E', NULL, " ");
					n_errors++;
				}
				Ctrl->E.active = true;
				break;
			case 'G':	/* Set Gray shade for polygon */
				Ctrl->G.active = true;
				if (gmt_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					gmt_fill_syntax (GMT, 'G', NULL, " ");
					n_errors++;
				}
				break;
			case 'L':	/* Draw the outline */
				Ctrl->L.active = true;
				break;
			case 'N':	/* Do not skip points outside border */
				Ctrl->N.active = true;
				break;
			case 'S':	/* Get symbol [and size] */
 				txt_b[0] = '\0';
				if ((c = strstr (opt->arg, "+f"))) {	/* Gave font directly */
					n_errors += gmt_getfont (GMT, &c[2], &(Ctrl->S.font));
					c[0] = '\0';	/* Temporarily chop off the font specification */
				}
 				if (opt->arg[0] == 'e' || opt->arg[0] == 'r') {
					strncpy (txt, &opt->arg[1], GMT_LEN256);
					n = 0; while (txt[n] && txt[n] != '/') n++; txt[n] = 0;
					Ctrl->S.scale = gmt_M_to_inch (GMT, txt);
					sscanf (strchr(&opt->arg[1],'/')+1, "%lf/%s", &Ctrl->S.confidence, txt_b);
					/* confidence scaling */
					Ctrl->S.conrad = sqrt (-2.0 * log (1.0 - Ctrl->S.confidence));
					if (txt_b[0]) Ctrl->S.font.size = gmt_convert_units (GMT, txt_b, GMT_PT, GMT_PT);
				}
				if (opt->arg[0] == 'n' || opt->arg[0] == 'x' ) Ctrl->S.scale = gmt_M_to_inch (GMT, &opt->arg[1]);
				if (opt->arg[0] == 'w' && strlen(opt->arg) > 3) {
					strncpy(txt, &opt->arg[1], GMT_LEN256);
					n=0; while (txt[n] && txt[n] != '/') n++; txt[n]=0;
					Ctrl->S.scale = gmt_M_to_inch (GMT, txt);
					sscanf(strchr(&opt->arg[1],'/')+1, "%lf", &Ctrl->S.wedge_amp);
				}
				switch (opt->arg[0]) {
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
						n_errors++;
						break;
				}
				if (c) c[0] = '+';	/* Restore font specification */
				break;
			case 'W':	/* Set line attributes */
				Ctrl->W.active = true;
				if (opt->arg && gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', NULL, " ", 0);
					n_errors++;
				}
				break;

			/* Illegal options */

		}
	}

	no_size_needed = (Ctrl->S.readmode == READ_ELLIPSE || Ctrl->S.readmode == READ_ROTELLIPSE || Ctrl->S.readmode == READ_ANISOTROPY || Ctrl->S.readmode == READ_CROSS || Ctrl->S.readmode == READ_WEDGE );
        /* Only one allowed */
	n_set = (Ctrl->S.readmode == READ_ELLIPSE) + (Ctrl->S.readmode == READ_ROTELLIPSE) + (Ctrl->S.readmode == READ_ANISOTROPY) + (Ctrl->S.readmode == READ_CROSS) + (Ctrl->S.readmode == READ_WEDGE);
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, n_set > 1, "Only one -S setting is allowed.\n");
	n_errors += gmt_M_check_condition (GMT, !no_size_needed && (Ctrl->S.symbol > 1 && Ctrl->S.scale <= 0.0), "Option -S: Must specify symbol size.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && ! (Ctrl->S.readmode == READ_ELLIPSE || Ctrl->S.readmode == READ_WEDGE), "Option -D requires -Se|w.\n");

	if (!got_A && Ctrl->W.active) Ctrl->A.S.v.pen = Ctrl->W.pen;	/* Set vector pen to that given by -W  */
	if (Ctrl->A.S.v.status & PSL_VEC_OUTLINE2 && Ctrl->W.active) gmt_M_rgb_copy (Ctrl->A.S.v.pen.rgb, Ctrl->W.pen.rgb);	/* Set vector pen color from -W but not thickness */
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_psvelo (void *V_API, int mode, void *args) {
	int ix = 0, iy = 1, n_rec = 0, justify;
	int des_ellipse = true, des_arrow = true, error = false;

	double plot_x, plot_y, vxy[2], plot_vx, plot_vy, length, s, dim[PSL_MAX_DIMS];
	double eps1 = 0.0, eps2 = 0.0, spin = 0.0, spinsig = 0.0, theta = 0.0, *in = NULL;
	double direction = 0, small_axis = 0, great_axis = 0, sigma_x, sigma_y, corr_xy;
	double t11 = 1.0, t12 = 0.0, t21 = 0.0, t22 = 1.0, hl, hw, vw, ssize, headpen_width = 0.0;

	char *station_name = NULL;

	struct GMT_RECORD *In = NULL;
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

	if (gmt_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_PROJECTION_ERROR);

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
	gmt_set_basemap_orders (GMT, Ctrl->N.active ? GMT_BASEMAP_FRAME_BEFORE : GMT_BASEMAP_FRAME_AFTER, GMT_BASEMAP_GRID_BEFORE, GMT_BASEMAP_ANNOT_AFTER);
	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */
	gmt_map_basemap (GMT);	/* Basemap before data */

	gmt_M_memset (dim, PSL_MAX_DIMS, double);
	gmt_setpen (GMT, &Ctrl->W.pen);
	PSL_setfont (PSL, GMT->current.setting.font_annot[GMT_PRIMARY].id);
	if (Ctrl->E.active) Ctrl->L.active = true;

	if (!Ctrl->N.active) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);
	gmt_init_vector_param (GMT, &Ctrl->A.S, true, Ctrl->W.active, &Ctrl->W.pen, Ctrl->G.active, &Ctrl->G.fill);
	if (Ctrl->A.S.symbol == PSL_VECTOR) Ctrl->A.S.v.v_width = (float)(Ctrl->A.S.v.pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);

	ix = (GMT->current.setting.io_lonlat_toggle[0]);	iy = 1 - ix;

	GMT_Set_Columns (API, GMT_IN, Ctrl->S.n_cols, GMT_COL_FIX);

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	if (Ctrl->S.readmode == READ_ELLIPSE || Ctrl->S.readmode == READ_ROTELLIPSE) GMT_Report (API, GMT_MSG_INFORMATION, "psvelo: 2-D confidence interval and scaling factor %f %f\n", Ctrl->S.confidence, Ctrl->S.conrad);

	if (Ctrl->S.symbol == CINE || Ctrl->S.symbol == CROSS) {
		if (Ctrl->A.S.v.status & PSL_VEC_OUTLINE2) {	/* Vector head outline pen specified separately */
			PSL_defpen (PSL, "PSL_vecheadpen", Ctrl->A.S.v.pen.width, Ctrl->A.S.v.pen.style, Ctrl->A.S.v.pen.offset, Ctrl->A.S.v.pen.rgb);
			headpen_width = 0.5*Ctrl->A.S.v.pen.width;
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

		in = In->data;
		station_name = In->text;

		/* Data record to process */

		n_rec++;

		if (Ctrl->S.readmode == READ_ELLIPSE) {
			vxy[ix] = in[2];
			vxy[iy] = in[3];
			sigma_x = in[4];
			sigma_y = in[5];
			corr_xy = in[6];
			/* rescale uncertainties if necessary */
			if (Ctrl->D.active) {
				sigma_x = Ctrl->D.scale * sigma_x;
				sigma_y = Ctrl->D.scale * sigma_y;
			}
			if (fabs (sigma_x) < EPSIL && fabs (sigma_y) < EPSIL)
				des_ellipse = false;
			else {
				des_ellipse = true;
				psvelo_ellipse_convert (sigma_x, sigma_y, corr_xy, Ctrl->S.conrad, &small_axis, &great_axis, &direction);

				/* convert to degrees */
				direction = direction * R2D;
			}
		}
		else if (Ctrl->S.readmode == READ_ROTELLIPSE) {
			vxy[ix] = in[2];
			vxy[iy] = in[3];
			great_axis = Ctrl->S.conrad*in[4];
			small_axis = Ctrl->S.conrad*in[5];
			direction = in[6];
			if (fabs (great_axis) < EPSIL && fabs (small_axis) < EPSIL)
				des_ellipse = false;
			else
				des_ellipse = true;
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
			if (Ctrl->D.active) spinsig = spinsig * Ctrl->D.scale;
		}

		if (!Ctrl->N.active) {
			gmt_map_outside (GMT, in[GMT_X], in[GMT_Y]);
			if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) continue;
		}

		gmt_geo_to_xy (GMT, in[GMT_X], in[GMT_Y], &plot_x, &plot_y);

		switch (Ctrl->S.symbol) {
			case CINE:
				des_arrow = hypot (vxy[0], vxy[1]) < 1.e-8 ? false : true;
				psvelo_trace_arrow (GMT, in[GMT_X], in[GMT_Y], vxy[0], vxy[1], Ctrl->S.scale, &plot_x, &plot_y, &plot_vx, &plot_vy);
				psvelo_get_trans (GMT, in[GMT_X], in[GMT_Y], &t11, &t12, &t21, &t22);
				if (des_ellipse) {
					if (Ctrl->E.active)
						psvelo_paint_ellipse (GMT, plot_vx, plot_vy, direction, great_axis, small_axis, Ctrl->S.scale,
							t11,t12,t21,t22, Ctrl->E.active, &Ctrl->E.fill, Ctrl->L.active);
					else
						psvelo_paint_ellipse (GMT, plot_vx, plot_vy, direction, great_axis, small_axis, Ctrl->S.scale,
							t11,t12,t21,t22, Ctrl->E.active, &Ctrl->G.fill, Ctrl->L.active);
				}
				if (des_arrow) {	/* verify that arrow is not ridiculously small */
					length = hypot (plot_x-plot_vx, plot_y-plot_vy);	/* Length of arrow */
					if (length < Ctrl->A.S.v.h_length && Ctrl->A.S.v.v_norm < 0.0)	/* No shrink requested yet head length exceeds total vector length */
						GMT_Report (API, GMT_MSG_INFORMATION, "Vector head length exceeds overall vector length near line %d. Consider adding +n<norm> to -A\n", n_rec);
					s = (length < Ctrl->A.S.v.v_norm) ? length / Ctrl->A.S.v.v_norm : 1.0;
					hw = s * Ctrl->A.S.v.h_width;
					hl = s * Ctrl->A.S.v.h_length;
					vw = s * Ctrl->A.S.v.v_width;
					if (vw < 2.0/PSL_DOTS_PER_INCH) vw = 2.0/PSL_DOTS_PER_INCH;	/* Minimum width set */
					if (Ctrl->A.S.v.status & PSL_VEC_OUTLINE2) gmt_setpen (GMT, &Ctrl->A.S.v.pen);
					dim[0] = plot_vx, dim[1] = plot_vy;
					dim[2] = vw, dim[3] = hl, dim[4] = hw;
					dim[5] = Ctrl->A.S.v.v_shape;
					if (Ctrl->A.S.symbol == GMT_SYMBOL_VECTOR_V4) {
						double *this_rgb = NULL;
						if (Ctrl->G.active)
							this_rgb = Ctrl->G.fill.rgb;
						else
							this_rgb = GMT->session.no_rgb;
						if (Ctrl->L.active) gmt_setpen (GMT, &Ctrl->W.pen);
						psl_vector_v4 (PSL, plot_x, plot_y, dim, this_rgb, Ctrl->L.active);
					}
					else {
						dim[6] = (double)Ctrl->A.S.v.status;
						dim[7] = (double)Ctrl->A.S.v.v_kind[0];	dim[8] = (double)Ctrl->A.S.v.v_kind[1];
						dim[11] = (headpen_width > 0.0) ? headpen_width : 0.5 * Ctrl->W.pen.width;
						if (Ctrl->A.S.v.status & PSL_VEC_FILL2)
							gmt_setfill (GMT, &Ctrl->A.S.v.fill, Ctrl->L.active);
						else if (Ctrl->G.active)
							gmt_setfill (GMT, &Ctrl->G.fill, Ctrl->L.active);
						PSL_plotsymbol (PSL, plot_x, plot_y, dim, PSL_VECTOR);
					}
					if (Ctrl->A.S.v.status & PSL_VEC_OUTLINE2) gmt_setpen (GMT, &Ctrl->W.pen);

					justify = plot_vx - plot_x > 0. ? PSL_MR : PSL_ML;
					if (Ctrl->S.font.size > 0.0 && station_name)	/* 1 inch = 2.54 cm */
						PSL_plottext (PSL, plot_x + (6 - justify) / 25.4 , plot_y, Ctrl->S.font.size, station_name, ANGLE, justify, FORM);
				}
				else {
					gmt_setfill (GMT, &Ctrl->G.fill, 1);
					ssize = GMT_DOT_SIZE;
					PSL_plotsymbol (PSL, plot_x, plot_y, &ssize, PSL_CIRCLE);
					justify = PSL_TC;
					if (Ctrl->S.font.size > 0.0 && station_name) {
						PSL_plottext (PSL, plot_x, plot_y - 1. / 25.4, Ctrl->S.font.size, station_name, ANGLE, justify, FORM);
					}
					/*  1 inch = 2.54 cm */
				}
				break;
			case ANISO:
				psvelo_trace_arrow (GMT, in[GMT_X], in[GMT_Y], vxy[0], vxy[1], Ctrl->S.scale, &plot_x, &plot_y, &plot_vx, &plot_vy);
				PSL_plotsegment (PSL, plot_x, plot_y, plot_vx, plot_vy);
				break;
			case CROSS:
				/* triangular arrowheads */
				psvelo_trace_cross (GMT, in[GMT_X],in[GMT_Y],eps1,eps2,theta,Ctrl->S.scale,Ctrl->A.S.v.v_width,Ctrl->A.S.v.h_length,
					Ctrl->A.S.v.h_width,0.1,Ctrl->L.active,&(Ctrl->W.pen));
				break;
			case WEDGE:
				PSL_comment (PSL, "begin wedge number %li", n_rec);
				gmt_geo_to_xy (GMT, in[GMT_X], in[GMT_Y], &plot_x, &plot_y);
				psvelo_get_trans (GMT, in[GMT_X], in[GMT_Y], &t11, &t12, &t21, &t22);
				psvelo_paint_wedge (PSL, plot_x, plot_y, spin, spinsig, Ctrl->S.scale, Ctrl->S.wedge_amp, t11,t12,t21,t22,
					Ctrl->G.active, Ctrl->G.fill.rgb, Ctrl->E.active, Ctrl->E.fill.rgb, Ctrl->L.active);
				break;
		}
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Number of records read: %li\n", n_rec);

	if (Ctrl->D.active)  GMT_Report (API, GMT_MSG_INFORMATION, "Rescaling uncertainties by a factor of %f\n", Ctrl->D.scale);

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
