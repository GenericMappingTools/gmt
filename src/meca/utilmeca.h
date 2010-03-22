/*	$Id: utilmeca.h,v 1.12 2010-03-22 18:55:46 guru Exp $
 *    Copyright (c) 1996-2010 by G. Patau
 *    Distributed under the GNU Public Licence
 *    See README file for copying and redistribution conditions.
 */

void get_trans (double slon,double slat,double *t11,double *t12,double *t21,double *t22);
double  ps_mechanism(double x0, double y0, st_me meca, double size, int rgb[3], int ergb[3], GMT_LONG outline);
double ps_meca(double x0,double y0,st_me meca,double size);
double ps_plan(double x0,double y0,st_me meca,double size,GMT_LONG num_of_plane);
double computed_mw(struct MOMENT moment,double ms);
double computed_dip2(double str1,double dip1,double str2);
double computed_rake2(double str1,double dip1,double str2,double dip2,double fault);
void define_second_plane(struct nodal_plane NP,struct nodal_plane *NP2);
double ps_tensor(double x0,double y0,double size,struct AXIS T,struct AXIS N,struct AXIS P,int c_rgb[3],int e_rgb[3], GMT_LONG outline, GMT_LONG plot_zerotrace);
void axe2dc(struct AXIS T,struct AXIS P,struct nodal_plane *NP1,struct nodal_plane *NP2);
void ps_pt_axis(double x0,double y0,st_me meca,double size,double *pp,double *dp,double *pt,double *dt,double *xp,double *yp,double *xt,double *yt);
void GMT_momten2axe(struct M_TENSOR mt,struct AXIS *T,struct AXIS *N,struct AXIS *P);
void axis2xy(double x0,double y0,double size,double pp,double dp,double pt,double dt,double *xp,double *yp,double *xt,double *yt);
