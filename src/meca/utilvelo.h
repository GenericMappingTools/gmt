/*	$Id: utilvelo.h,v 1.10 2010-03-22 18:55:46 guru Exp $
 *    Copyright (c) 1996-2010 by G. Patau
 *    Distributed under the GNU Public Licence
 *    See README file for copying and redistribution conditions.
 */

void get_trans (double slon,double slat,double *t11,double *t12,double *t21,double *t22);
void transform_local (double x0,double y0,double dxp,double dyp,double scale,double t11,double t12,double t21,double t22,double *x1,double *y1);
void trace_arrow (double slon,double slat,double dxp,double dyp,double scale,double *x1,double *y1,double *x2,double *y2);
void ellipse_convert (double sigx,double sigy,double rho,double conrad,double *eigen1,double *eigen2,double *ang) ;
void paint_ellipse (double x0, double y0, double angle, double major, double minor, double scale, double t11,double t12,double t21,double t22, GMT_LONG polygon, int rgb[3], GMT_LONG outline);
