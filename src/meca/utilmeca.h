/*	$Id: utilmeca.h,v 1.14 2011-03-15 02:06:37 guru Exp $
 *    Copyright (c) 1996-2011 by G. Patau
 *    Distributed under the GNU Public Licence
 *    See README file for copying and redistribution conditions.
 */

void get_trans (struct GMT_CTRL *GMT, double slon, double slat, double *t11, double *t12, double *t21, double *t22);
double ps_mechanism (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x0, double y0, st_me meca, double size, struct GMT_FILL *F, struct GMT_FILL *E, GMT_LONG outline);
double ps_meca (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x0, double y0, st_me meca, double size);
double ps_plan (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x0, double y0, st_me meca, double size, GMT_LONG num_of_plane);
double computed_mw(struct MOMENT moment, double ms);
double computed_dip2(double str1, double dip1, double str2);
double computed_rake2(double str1, double dip1, double str2, double dip2, double fault);
void define_second_plane(struct nodal_plane NP, struct nodal_plane *NP2);
double ps_tensor (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x0, double y0, double size, struct AXIS T, struct AXIS N, struct AXIS P, struct GMT_FILL *C, struct GMT_FILL *E, GMT_LONG outline, GMT_LONG plot_zerotrace);
void axe2dc(struct AXIS T, struct AXIS P, struct nodal_plane *NP1, struct nodal_plane *NP2);
void ps_pt_axis(double x0, double y0, st_me meca, double size, double *pp, double *dp, double *pt, double *dt, double *xp, double *yp, double *xt, double *yt);
void GMT_momten2axe(struct GMT_CTRL *GMT, struct M_TENSOR mt, struct AXIS *T, struct AXIS *N, struct AXIS *P);
void axis2xy(double x0, double y0, double size, double pp, double dp, double pt, double dt, double *xp, double *yp, double *xt, double *yt);
void transform_local (double x0, double y0, double dxp, double dyp, double scale, double t11, double t12, double t21, double t22, double *x1, double *y1);
void trace_arrow (struct GMT_CTRL *GMT, double slon, double slat, double dxp, double dyp, double scale, double *x1, double *y1, double *x2, double *y2);
void ellipse_convert (double sigx, double sigy, double rho, double conrad, double *eigen1, double *eigen2, double *ang) ;
void paint_ellipse (struct PSL_CTRL *PSL, double x0, double y0, double angle, double major, double minor, double scale, double t11, double t12, double t21, double t22, GMT_LONG polygon, double *rgb, GMT_LONG outline);
GMT_LONG trace_cross (struct GMT_CTRL *GMT, double slon, double slat, double eps1, double eps2, double theta, double sscale, double v_width, double h_length, double h_width, double vector_shape, GMT_LONG outline, struct GMT_PEN pen);
void paint_wedge (struct PSL_CTRL *PSL, double x0, double y0, double spin, double spinsig, double sscale, double wedge_amp, double t11, double t12, double t21, double t22, GMT_LONG polygon, double *rgb, GMT_LONG epolygon, double *ergb, GMT_LONG outline);
