/*	$Id: utilvelo.c,v 1.10 2009-05-08 14:51:14 remko Exp $
 *    Copyright (c) 1996-2009 by G. Patau
 *    Distributed under the GNU Public Licence
 *    See README file for copying and redistribution conditions.
 */

#include "gmt.h"	/* to have gmt environment */
#include "pslib.h"	/* to have pslib environment */
#include <math.h>

#define squared(x) ((x) * (x))

/************************************************************************/
void get_trans (double slon,double slat,double *t11,double *t12,double *t21,double *t22)

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

     {                          
                                                                                    

     /* LOCAL VARIABLES */                                                          
     double su,sv,udlat,vdlat,udlon,vdlon,dudlat,dvdlat,dudlon,dvdlon;
     double dl;

     /* how much does x,y change for a 1 degree change in lon,lon ? */
     GMT_geo_to_xy (slon,     slat,     &su,    &sv );
     GMT_geo_to_xy (slon,     slat+1.0, &udlat, &vdlat);
     GMT_geo_to_xy (slon+1.0, slat    , &udlon, &vdlon);

     /* Compute dudlat, dudlon, dvdlat, dvdlon */
     dudlat = udlat - su;
     dvdlat = vdlat - sv;
     dudlon = udlon - su;
     dvdlon = vdlon - sv;

     /* Make unit vectors for the long (e/x) and lat (n/y) */
     /* to construct local transformation matrix */

     dl = sqrt( dudlon*dudlon + dvdlon*dvdlon );
     *t11 = dudlon/dl ;
     *t21 = dvdlon/dl ;

     dl = sqrt( dudlat*dudlat + dvdlat*dvdlat );
     *t12 = dudlat/dl ;
     *t22 = dvdlat/dl ;
	   }

/************************************************************************/
void transform_local (double x0,double y0,double dxp,double dyp,double scale,double t11,double t12,double t21,double t22,double *x1,double *y1)

     /* perform local transformation on offsets (dxp,dyp) from */
     /* "origin point" x0,y0 given transformation matrix T */

     /* Kurt Feigl, from code by T. Herring */                                       
                                                                                    
     {                          
                                                          
                                                                                    
     /* INPUT */                                                                    
     /*   x0,y0       - dxp,dyp with respect to this point */
     /*   dxp         - x component of arrow */
     /*   dyp         - y component of arrow */
     /*   scale       - scaling for arrow    */
     /*   t11,t12,t21,t22 transformation matrix */
                                                                                    
     /* OUTPUT (returned) */                                                        
     /*   x1,y1       - paper coordinates of arrow tail */
                                                                                    
     /* LOCAL VARIABLES */                                                          
     double du,dv;

     /* perform local transformation */
     du = scale * (t11*dxp + t12*dyp);
     dv = scale * (t21*dxp + t22*dyp);

     /*  Now add to origin  and return values */
     *x1 = x0 + du;
     *y1 = y0 + dv;

	   }

/************************************************************************/
void trace_arrow (double slon,double slat,double dxp,double dyp,double scale,double *x1,double *y1,double *x2,double *y2)

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

     {                          

     /* local */
     double t11,t12,t21,t22,xt,yt;

     /* determine local transformation between (lon,lat) and (x,y) */
     /* return this in the 2 x 2 matrix t */
     get_trans (slon,slat,&t11,&t12,&t21,&t22);

     /* map start of arrow from lat,lon to x,y */
     GMT_geo_to_xy (slon, slat, &xt, &yt);

     /* perform the transformation */
     transform_local (xt,yt,dxp,dyp,scale,t11,t12,t21,t22,x2,y2);

     /* return values */

     *x1 = xt;
     *y1 = yt;

	   }

/*********************************************************************/
void trace_ellipse (double angle, double major, double minor, GMT_LONG npoints, double *x, double *y)

/* Given specs for an ellipse, return it in x,y */

{
    double phi = 0.;
    double sd, cd, s, c;

    int i;

    sincos (angle*M_PI/180., &sd, &cd);

    for(i = 0; i < 360; i++) {
    sincos (phi, &s, &c);
        *x++ = major * c * cd - minor * s * sd;
        *y++ = major * c * sd + minor * s * cd;
        phi += M_PI*2./(npoints-2);
    } 
}

/********************************************************************/
void ellipse_convert (double sigx,double sigy,double rho,double conrad,double *eigen1,double *eigen2,double *ang) 

     /* convert from one parameterization of an ellipse to another */

     /* Kurt Feigl, from code by T. Herring */                                       
                                                                                    
     {                          

     /* INPUT */                                                                    
     /*   sigx, sigy  - Sigmas in the x and y directions. */                      
     /*   rho         - Correlation coefficient between x and y */                  
                                                                                    
     /* OUTPUT (returned) */                                                        
     /*   eigen1      - the smaller eigenvalue */
     /*   eigen2      - the larger eigenvalue */
     /*   ang         - Orientation of ellipse relative to X axis in radians */     
     /*               - should be counter-clockwise from X axis */
                                                                                    
     /* LOCAL VARIABLES */                                                          
                                                                                    
     /*   a,b,c,d,e   - Constants used in getting eigenvalues */                    
     /*   conrad      - Radius for the confidence interval */                       

      double a,b,c,d,e;

      /* confidence scaling */
      /*   confid      - Confidence interval wanted (0-1) */                         
      /* conrad = sqrt( -2.0 * log(1.0 - confid)); */

      /* the formulas for this part may be found in Bomford, p. 719 */

      a = squared(sigy*sigy - sigx*sigx);
      b = 4. * squared(rho*sigx*sigy);
      c = squared(sigx) + squared(sigy);

      /* minimum eigenvector (semi-minor axis) */
      *eigen1 = conrad * sqrt((c - sqrt(a + b))/2.);

      /* maximu eigenvector (semi-major axis) */
      *eigen2 = conrad * sqrt((c + sqrt(a + b))/2.);

      d = 2. * rho * sigx * sigy;
      e = squared(sigx) - squared(sigy);

      *ang = atan2(d,e)/2.;

/*    that is all */

}

/********************************************************************/
void paint_ellipse (double x0, double y0, double angle, double major, double minor, double scale, double t11,double t12,double t21,double t22, int polygon, int rgb[3], BOOLEAN outline)

/* Make an ellipse at center x0,y0  */

{     

#define NPOINTS 362

     GMT_LONG    npoints = NPOINTS;
     int    i;
     /* relative to center of ellipse */
     double dxe[NPOINTS],dye[NPOINTS];
     /* absolute paper coordinates */
     double axe[NPOINTS],aye[NPOINTS];

     trace_ellipse(angle, major, minor, npoints, dxe, dye);

     for (i = 0; i < npoints - 2; i++) {
         transform_local (x0,y0,dxe[i],dye[i],scale,t11,t12,t21,t22,&axe[i],&aye[i]);
     }    
     if(polygon)
         ps_polygon(axe, aye, npoints - 2, rgb, outline);
     else	
         ps_line(axe, aye, npoints - 2, 3, TRUE);

}
