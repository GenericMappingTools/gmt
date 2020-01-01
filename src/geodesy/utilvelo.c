/*
 *    Copyright (c) 1996-2012 by G. Patau
 *    Copyright (c) 2013-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *    Donated to the GMT project by G. Patau upon her retirement from IGPG
 *    Distributed under the Lesser GNU Public Licence
 *    See README file for copying and redistribution conditions.
 */

#include "gmt_dev.h"	/* to have gmt dev environment */
#include "utilvelo.h"

#define squared(x) ((x) * (x))

/************************************************************************/
void velo_get_trans (struct GMT_CTRL *GMT, double slon, double slat, double *t11, double *t12, double *t21, double *t22) {
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

static void transform_local (double x0, double y0, double dxp, double dyp, double scale, double t11, double t12, double t21, double t22, double *x1, double *y1) {
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

void velo_trace_arrow (struct GMT_CTRL *GMT, double slon, double slat, double dxp, double dyp, double scale, double *x1, double *y1, double *x2, double *y2) {
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
	velo_get_trans (GMT, slon, slat, &t11, &t12, &t21, &t22);

	/* map start of arrow from lat, lon to x, y */
	gmt_geo_to_xy (GMT, slon, slat, &xt, &yt);

	/* perform the transformation */
	transform_local (xt, yt, dxp, dyp, scale, t11, t12, t21, t22, x2, y2);

	/* return values */

	*x1 = xt;
	*y1 = yt;
}

static void trace_ellipse (double angle, double major, double minor, int npoints, double *x, double *y) {
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

void velo_ellipse_convert (double sigx, double sigy, double rho, double conrad, double *eigen1, double *eigen2, double *ang) {
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

void velo_paint_ellipse (struct GMT_CTRL *GMT, double x0, double y0, double angle, double major, double minor, double scale, double t11, double t12, double t21, double t22, int polygon, struct GMT_FILL *fill, int outline) {
	/* Make an ellipse at center x0,y0  */
#define NPOINTS_ELLIPSE 362

	int npoints = NPOINTS_ELLIPSE, i;
	/* relative to center of ellipse */
	double dxe[NPOINTS_ELLIPSE],dye[NPOINTS_ELLIPSE];
	/* absolute paper coordinates */
	double axe[NPOINTS_ELLIPSE],aye[NPOINTS_ELLIPSE];

	trace_ellipse (angle, major, minor, npoints, dxe, dye);

	for (i = 0; i < npoints - 2; i++) transform_local (x0, y0, dxe[i], dye[i], scale, t11, t12, t21, t22, &axe[i], &aye[i]);
	if (polygon) {
		gmt_setfill (GMT, fill, outline);
		PSL_plotpolygon (GMT->PSL, axe, aye, npoints - 2);
	}
	else
		PSL_plotline (GMT->PSL, axe, aye, npoints - 2, PSL_MOVE|PSL_STROKE|PSL_CLOSE);
}

/************************************************************************/
int velo_trace_cross (struct GMT_CTRL *GMT, double slon, double slat, double eps1, double eps2, double theta, double sscale, double v_width, double h_length, double h_width, double vector_shape, int outline, struct GMT_PEN *pen) {
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
	velo_trace_arrow (GMT, slon, slat, dx, dy, sscale, &x1, &y1, &x2, &y2);

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

	velo_trace_arrow (GMT, slon, slat, -dx, -dy, sscale, &x1, &y1, &x2, &y2);

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
	velo_trace_arrow (GMT, slon, slat, dx, dy, sscale, &x1, &y1, &x2, &y2);

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

	velo_trace_arrow (GMT, slon, slat, -dx, -dy, sscale, &x1, &y1, &x2, &y2);

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

static int trace_wedge (double spin, double sscale, double wedge_amp, int lines, double *x, double *y) {
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

static int trace_sigwedge (double spin, double spinsig, double sscale, double wedge_amp, double *x, double *y) {
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

void velo_paint_wedge (struct PSL_CTRL *PSL, double x0, double y0, double spin, double spinsig, double sscale, double wedge_amp, double t11, double t12, double t21, double t22, int polygon, double *rgb, int epolygon, double *ergb, int outline) {

	/* Make a wedge at center x0,y0  */

#define NPOINTS 1000

	int npoints = NPOINTS, i;

	/* relative to center of ellipse */
	double dxe[NPOINTS], dye[NPOINTS];
	/* absolute paper coordinates */
	double axe[NPOINTS], aye[NPOINTS];
	gmt_M_unused(outline);

	/* draw wedge */

	npoints = trace_wedge (spin, 1.0, wedge_amp, true, dxe, dye);

	for (i = 0; i <= npoints - 1; i++)
		transform_local (x0, y0, dxe[i], dye[i], sscale, t11, t12, t21, t22, &axe[i], &aye[i]);

	if (polygon) {
		PSL_setfill (PSL, rgb, true);
		PSL_plotpolygon (PSL, axe, aye, npoints);
	}
	else
		PSL_plotline (PSL, axe, aye, npoints, PSL_MOVE|PSL_STROKE|PSL_CLOSE);

	/* draw uncertainty wedge */

	npoints = trace_sigwedge (spin, spinsig, 1.0,wedge_amp, dxe, dye);

	for (i = 0; i < npoints - 1; i++) transform_local (x0, y0, dxe[i], dye[i], sscale, t11, t12, t21, t22, &axe[i], &aye[i]);

	if (epolygon) {
		PSL_setfill (PSL, ergb, true);
		PSL_plotpolygon (PSL, axe, aye, npoints - 1);
	}
	else
		PSL_plotline (PSL, axe, aye, npoints - 1, PSL_MOVE|PSL_STROKE|PSL_CLOSE);
}
