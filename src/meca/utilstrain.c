/*	$Id: utilstrain.c,v 1.9 2009-01-09 04:02:35 guru Exp $
 *    Copyright (c) 1996-2009 by G. Patau
 *    Distributed under the GNU Public Licence
 *    See README file for copying and redistribution conditions.
 */

#include "gmt.h"	/* to have gmt environment */
#include "pslib.h"	/* to have pslib environment */
#include "utilvelo.h"
#define veclen(x, y) sqrt((x) * (x) + (y) * (y))

/************************************************************************/
int trace_cross (double slon,double slat,double eps1,double eps2,double theta,double sscale,double v_width,double h_length,double h_width,double vector_shape,BOOLEAN outline,struct GMT_PEN pen)

     /* make a Strain rate cross at(slat,slon) */

     /* Kurt Feigl, from code by D. Dong */                                       


     /*   INPUT VARIABLES: */
     /*   slat        - latitude, in degrees of arrow tail */
     /*   slon        - longitude in degrees of arrow tail */
     /*   sscale      : scaling factor for size of cloverleaf */
     /*   theta       : azimuth of more compressive eigenvector (deg) */
     /*   eps1,eps2   : eigenvalues of strain rate (1/yr) */
     /*   v_width, h_length,h_width,vector_shape: arrow characteristics */
                
     {                          

     /* local */
     double dx,dy,xh,yh,x1,x2,y1,y2,hl,hw,vw;
     double s,c;

    sincos (theta*M_PI/180., &s, &c);

    /*  extension component */
    dx =  eps1 * c;
    dy = -eps1 * s;

    /* arrow is outward from slat,slon */
    trace_arrow(slon,slat,dx,dy, sscale, &x1, &y1, &x2, &y2);

    if (eps1 < 0.) {
      xh = x1;
      yh = y1;
      x1 = x2;
      y1 = y2;
      x2 = xh;
      y2 = yh;
    }

    if (veclen (x1-x2,y1-y2) <= 1.5 * h_length) {
      hl = veclen (x1-x2,y1-y2) * 0.6;
      hw = hl * h_width/h_length;
      vw = hl * v_width/h_length; 
      if (vw < 2./(double)gmtdefs.dpi) vw = 2./(double)gmtdefs.dpi;
      }
    else {
     hw = h_width;
     hl = h_length;
     vw = v_width;
     }
  
    ps_vector(x1, y1, x2, y2, vw, hl, hw, vector_shape, pen.rgb, outline);

    /* second, extensional arrow in opposite direction */

    trace_arrow(slon,slat,-dx,-dy, sscale, &x1, &y1, &x2, &y2);

    if (eps1 < 0.) {
      xh = x1;
      yh = y1;
      x1 = x2;
      y1 = y2;
      x2 = xh;
      y2 = yh;
    }

    if (veclen (x1-x2,y1-y2) <= 1.5 * h_length) {
      hl = veclen (x1-x2,y1-y2) * 0.6;
      hw = hl * h_width/h_length;
      vw = hl * v_width/h_length; 
      if (vw < 2./(double)gmtdefs.dpi) vw = 2./(double)gmtdefs.dpi;
      }
    else {
     hw = h_width;
     hl = h_length;
     vw = v_width;
     }

    ps_vector(x1, y1, x2, y2, vw, hl, hw, vector_shape, pen.rgb, outline);

   /* compression component */
    dx = eps2 * s;
    dy = eps2 * c;

    trace_arrow(slon,slat,dx,dy, sscale, &x1, &y1, &x2, &y2);

    if (eps2 > 0.) {
      xh = x1;
      yh = y1;
      x1 = x2;
      y1 = y2;
      x2 = xh;
      y2 = yh;
    }

    /* arrow should go toward slat, slon */
    if (veclen (x1-x2,y1-y2) <= 1.5 * h_length) {
      hl = veclen (x1-x2,y1-y2) * 0.6;
      hw = hl * h_width/h_length;
      vw = hl * v_width/h_length; 
      if (vw < 2./(double)gmtdefs.dpi) vw = 2./(double)gmtdefs.dpi;
      }
    else {
     hw = h_width;
     hl = h_length;
     vw = v_width;
     }

    ps_vector(x2, y2, x1, y1, vw, hl, hw, vector_shape, pen.rgb, outline);

    /* second, compressional arrow in opposite direction */

    trace_arrow(slon,slat,-dx,-dy, sscale, &x1, &y1, &x2, &y2);

    if (eps2 > 0.) {
      xh = x1;
      yh = y1;
      x1 = x2;
      y1 = y2;
      x2 = xh;
      y2 = yh;
    }

    /* arrow should go toward slat, slon */

    if (veclen (x1-x2,y1-y2) <= 1.5 * h_length) {
      hl = veclen (x1-x2,y1-y2) * 0.6;
      hw = hl * h_width/h_length;
      vw = hl * v_width/h_length; 
      if (vw < 2./(double)gmtdefs.dpi) vw = 2./(double)gmtdefs.dpi;
      }
    else {
     hw = h_width;
     hl = h_length;
     vw = v_width;
     }

    ps_vector(x2, y2, x1, y1, vw, hl, hw, vector_shape,   pen.rgb, outline);

    return 0;
} 

/************************************************************************/
int trace_wedge1 (double spin,double spinsig,double sscale,double wedge_amp,double *x,double *y)

     /* make a rotation rate wedge and return in x,y */

     /* Kurt Feigl, from code by D. Dong */                                       


     /*   INPUT VARIABLES: */
     /*   slat        - latitude, in degrees of arrow tail */
     /*   slon        - longitude in degrees of arrow tail */
     /*   sscale      : scaling factor for size (radius) of wedge */
     /*   wedge_amp   : scaling factor for angular size of wedge */
     /*   spin,spinsig :    :CW rotation rate and sigma in rad/yr */
     
     {                          

     int nstep,i1,i;
     double th,x0,y0,spin10,sig10,th0;
     double x1,y1;
     double s, c;
     int nump;

/*     How far would we spin */
      spin10 = wedge_amp * spin;
      sig10  = wedge_amp * spinsig;

/*     set origin */
      x0 = 0.;
      y0 = 0.;
      th0 = 0.;

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
        if (fabs(th-th0) >= 0.2) {
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
/*        go to zero and come back */
        ++nump;
        *x++ = x0;
        *y++ = y0;
        ++nump;
        *x++ = x1;
        *y++ = y1;
    }
    return nump;
}

/************************************************************************/

int trace_wedge (double spin,double sscale,double wedge_amp,int lines,double *x,double *y)

     /* make a rotation rate wedge and return in x,y */

     /* Kurt Feigl, from code by D. Dong */                                       

     /*   INPUT VARIABLES: */
     /*   slat        - latitude, in degrees of arrow tail */
     /*   slon        - longitude in degrees of arrow tail */
     /*   sscale      : scaling factor for size (radius) of wedge */
     /*   wedge_amp   : scaling factor for angular size of wedge */
     /*   spin        : CW spin rate in rad/yr */
     /*   lines :     : if true, draw lines                  */
     
     {                          

     int nstep,i1,i;
     double th,x0,y0,spin10,th0;
     double x1,y1;
     double s, c;
     int nump;

/*     How far would we spin */
      spin10 = wedge_amp * spin;

      th0 = 0.;      
       
/*     set origin */
      x0 = 0.;
      y0 = 0.;

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
        if (lines && fabs(th-th0) >= 0.2) {
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

/************************************************************************/
int trace_sigwedge (double spin,double spinsig,double sscale,double wedge_amp,double *x,double *y)


     /* make a rotation rate uncertainty wedge and return in x,y */

     /* Kurt Feigl, from code by D. Dong */                                       


     /*   INPUT VARIABLES: */
     /*   slat        - latitude, in degrees of arrow tail */
     /*   slon        - longitude in degrees of arrow tail */
     /*   sscale      : scaling factor for size (radius) of wedge */
     /*   wedge_amp   : scaling factor for angular size of wedge */
     /*   spin,spinsig :    :CW rotation rate and sigma in rad/yr */
     
     {                          

     int nstep, i;
     double th,x0,y0,spin10,sig10,th0;
     double x1,y1;
     double s,c;
     int nump;

/*     How far would we spin */
      spin10 = wedge_amp * spin;
      sig10  = wedge_amp * spinsig;

/*     set origin */
      x0 = 0.;
      y0 = 0.;
      th0 = 0.;

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

/********************************************************************/

void paint_wedge (double x0, double y0, double spin, double spinsig, double sscale, double wedge_amp, double t11,double t12,double t21,double t22,
		  int polygon, int rgb[3],
                  int epolygon, int ergb[3],
		  int outline)

/* Make a wedge at center x0,y0  */

{

#define NPOINTS 1000

     GMT_LONG    npoints = NPOINTS;
     int  i;

     /* relative to center of ellipse */
     double dxe[NPOINTS],dye[NPOINTS];
     /* absolute paper coordinates */
     double axe[NPOINTS],aye[NPOINTS];

     /*   draw wedge */

     npoints = trace_wedge (spin, 1., wedge_amp, TRUE, dxe, dye); 

     for (i = 0; i <= npoints - 1; i++) {
         transform_local (x0,y0,dxe[i],dye[i],sscale,t11,t12,t21,t22,&axe[i],&aye[i]);
     }

     if(polygon)
         ps_polygon(axe, aye, npoints, rgb, TRUE);
     else	
         ps_line(axe, aye, npoints, 3, FALSE, FALSE);

     /*   draw uncertainty wedge */

     npoints = trace_sigwedge (spin,spinsig, 1.,wedge_amp, dxe, dye); 

     for (i = 0; i < npoints - 1; i++) {
         transform_local (x0,y0,dxe[i],dye[i],sscale,t11,t12,t21,t22,&axe[i],&aye[i]);
     }

     if(epolygon)
         ps_polygon(axe, aye, npoints - 1, ergb, TRUE);
     else	
         ps_line(axe, aye, npoints - 1, 3, FALSE, FALSE);
}



