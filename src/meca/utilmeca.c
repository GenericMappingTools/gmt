/*	$Id: utilmeca.c,v 1.15 2009-02-06 22:55:44 guru Exp $
 *    Copyright (c) 1996-2009 by G. Patau
 *    Distributed under the GNU Public Licence
 *    See README file for copying and redistribution conditions.
 */

#include "gmt.h"	/* to have gmt environment */
#include "pslib.h"	/* to have pslib environment */
#include "meca.h"
#include "nrutil.h"

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

/***********************************************************************************************************/
double  ps_mechanism(double x0, double y0, st_me meca, double size, int rgb[3], int ergb[3], BOOLEAN outline)

/* Genevieve Patau */

{

    double null_axis_dip();
    double null_axis_strike();
    double proj_radius();

    double x[1000], y[1000];
    double pos_NP1_NP2 = sind(meca.NP1.str - meca.NP2.str);
    double fault = (GMT_IS_ZERO (meca.NP1.rake) ? meca.NP2.rake / fabs(meca.NP2.rake) : meca.NP1.rake / fabs(meca.NP1.rake));
    double radius_size;
    double str, radius, increment;
    double si, co;

	int lineout = 1, i;
	GMT_LONG npoints;

    struct AXIS N_axis;

/* compute null axis strike and dip */
    N_axis.dip = null_axis_dip(meca.NP1.str, meca.NP1.dip, meca.NP2.str, meca.NP2.dip);
    if (fabs(90. - N_axis.dip) < EPSIL) 
        N_axis.str = meca.NP1.str;
    else
        N_axis.str = null_axis_strike(meca.NP1.str, meca.NP1.dip, meca.NP2.str, meca.NP2.dip);
 
/* compute radius size of the bubble */
    radius_size = size * 0.5;

/* outline the bubble */
    ps_plot(x0 + radius_size, y0, 3); 
/*  argument is DIAMETER!!*/
    ps_circle(x0, y0, radius_size*2., ergb, lineout); 
 
    if (fabs(pos_NP1_NP2) < EPSIL) {
/* pure normal or inverse fault (null axis strike is determined
   with + or - 180 degrees. */
        /* first nodal plane part */
        i = -1;
        increment = 1.;
        str = meca.NP1.str;
        while (str <= meca.NP1.str + 180. + EPSIL) {
            i++;
            radius = proj_radius(meca.NP1.str, meca.NP1.dip, str) * radius_size;
            sincosd (str, &si, &co);
            x[i] = x0 + radius * si;
            y[i] = y0 + radius * co;
            str += increment;
        }
        if (fault < 0.) {
            /* normal fault, close first compressing part */
            str = meca.NP1.str + 180.; 
            while (str >= meca.NP1.str - EPSIL) {
                i++;
                sincosd (str, &si, &co);
                x[i] = x0 + si * radius_size;
                y[i] = y0 + co * radius_size;
                str -= increment;
            }
            npoints = i + 1;
            ps_polygon (x, y, npoints, rgb, outline);
            i = -1;
        }
        /* second nodal plane part */
        str = meca.NP2.str;
        while (str <= meca.NP2.str + 180. + EPSIL) {
            i++;
            radius = proj_radius(meca.NP2.str, meca.NP2.dip, str) * radius_size;
            sincosd (str, &si, &co);
            x[i] = x0 + radius * si;
            y[i] = y0 + radius * co;
            str += increment;
        }
        if (fault > 0.) {
            /* inverse fault, close compressing part */
            npoints = i+1;
            ps_polygon(x, y, npoints, rgb, outline);
        }
        else {
            /* normal fault, close second compressing part */
            str = meca.NP2.str + 180.;
            while (str >= meca.NP2.str - EPSIL) {
                i++;
                sincosd (str, &si, &co);
                x[i] = x0 + si * radius_size;
                y[i] = y0 + co * radius_size;
                str -= increment;
            } 
            npoints = i + 1;
            ps_polygon(x, y, npoints, rgb, outline);
        }
    }
/* pure strike-slip */
    else if ((90. - meca.NP1.dip) < EPSIL && (90. - meca.NP2.dip) < EPSIL) {
        increment = fabs(meca.NP1.rake) < EPSIL ? 1. : -1.;
        /* first compressing part */
        i = 0; 
        str = meca.NP1.str;
        while (increment > 0. ? str <= meca.NP1.str + 90. : str >= meca.NP1.str - 90.) {
            sincosd (str, &si, &co);
            x[i] = x0 + si * radius_size;
            y[i] = y0 + co * radius_size;
            str += increment;
            i++;
        }
        x[i] = x0;
        y[i] = y0;
        npoints = i + 1;
        ps_polygon(x, y, npoints, rgb, outline);
        /* second compressing part */
        i = 0;
        str = meca.NP1.str + 180.;
        while (increment > 0. ?  str <= meca.NP1.str + 270. + EPSIL : str >= meca.NP1.str + 90. - EPSIL) {
            sincosd (str, &si, &co);
            x[i] = x0 + si * radius_size;
            y[i] = y0 + co * radius_size;
            str += increment;
            i++;
        }
        x[i] = x0;
        y[i] = y0;
        npoints = i + 1;
        ps_polygon(x, y, npoints, rgb, outline);
    }
    else {
/* other cases */
        /* first nodal plane till null axis */
        i = -1;
        increment = 1.;
        if (meca.NP1.str > N_axis.str)
            meca.NP1.str -= 360.;
        str = meca.NP1.str;
        while (fabs(90. - meca.NP1.dip) < EPSIL ? 
	      str <= meca.NP1.str + EPSIL : str <= N_axis.str + EPSIL) {
            i++;
            radius = proj_radius(meca.NP1.str, meca.NP1.dip, str) * radius_size;
            sincosd (str, &si, &co);
            x[i] = x0 + radius * si;
            y[i] = y0 + radius * co;
            str += increment;
        }    
        
        /* second nodal plane from null axis */ 
        meca.NP2.str += (1. + fault) * 90.;
        if (meca.NP2.str >= 360.) meca.NP2.str -= 360.;
        increment = fault;
        if (fault * (meca.NP2.str - N_axis.str) < -EPSIL) meca.NP2.str += fault * 360.;
        str = fabs(90. - meca.NP2.dip) < EPSIL ? meca.NP2.str : N_axis.str;
        while (increment > 0. ? str <= meca.NP2.str + EPSIL : str >= meca.NP2.str - EPSIL) {
            i++;
            radius = proj_radius(meca.NP2.str - (1. + fault) * 90., meca.NP2.dip, str) * radius_size;
            sincosd (str, &si, &co);
            x[i] = x0 + radius * si;
            y[i] = y0 + radius * co;
            str += increment;
        }

        /* close the first compressing part */
        meca.NP1.str = zero_360(meca.NP1.str);
        meca.NP2.str = zero_360(meca.NP2.str);
        increment = pos_NP1_NP2 >= 0. ? -fault : fault;
        if (increment * (meca.NP1.str - meca.NP2.str) < - EPSIL) meca.NP1.str += increment * 360.;
        str = meca.NP2.str;
        while (increment > 0. ? str <= meca.NP1.str + EPSIL : str >= meca.NP1.str - EPSIL) {
            i++;
            sincosd (str, &si, &co);
            x[i] = x0 + si * radius_size;
            y[i] = y0 + co * radius_size;
            str += increment;
        }

        npoints = i + 1;
        ps_polygon(x, y, npoints, rgb, outline);

        /* first nodal plane till null axis */
        i = -1;
        meca.NP1.str = zero_360(meca.NP1.str + 180.);
        if (meca.NP1.str - N_axis.str < - EPSIL) meca.NP1.str += 360.;
        increment = -1.;
        str = meca.NP1.str;
        while (fabs(90. - meca.NP1.dip) < EPSIL ? str >= meca.NP1.str -EPSIL : str >= N_axis.str - EPSIL) {
            i++;
            radius = proj_radius(meca.NP1.str - 180., meca.NP1.dip, str) * radius_size;
            sincosd (str, &si, &co);
            x[i] = x0 + radius * si;
            y[i] = y0 + radius * co;
            str += increment;
        }

        /* second nodal plane from null axis */
        meca.NP2.str = zero_360(meca.NP2.str + 180.);
        increment = -fault;
        if (fault * (N_axis.str - meca.NP2.str) < - EPSIL) meca.NP2.str -= fault * 360.; 
        str = fabs(90. - meca.NP2.dip) < EPSIL ? meca.NP2.str : N_axis.str;
        while (increment > 0. ? str <= meca.NP2.str + EPSIL : str >= meca.NP2.str - EPSIL) {
            i++;
            radius = proj_radius(meca.NP2.str - (1. - fault) * 90., meca.NP2.dip, str) * radius_size;
            sincosd (str, &si, &co);
            x[i] = x0 + radius * si;
            y[i] = y0 + radius * co;
            str += increment;
        }

        /* close the second compressing part */
        meca.NP1.str = zero_360(meca.NP1.str);
        meca.NP2.str = zero_360(meca.NP2.str);
        increment = pos_NP1_NP2 >= 0. ? -fault : fault;
        if (increment * (meca.NP1.str - meca.NP2.str) < - EPSIL) meca.NP1.str += increment * 360.;
        str = meca.NP2.str;
        while (increment > 0. ? str <= meca.NP1.str + EPSIL : str >= meca.NP1.str - EPSIL) {
            i++;
            sincosd (str, &si, &co);
            x[i] = x0 + si * radius_size;
            y[i] = y0 + co * radius_size;
            str += increment;
        }

        npoints = i + 1;
        ps_polygon(x, y, npoints, rgb, outline);
    } 
    return(radius_size*2.);
} 

/*********************************************************************/
double ps_meca(double x0,double y0,st_me meca,double size)

/* Genevieve Patau */

{

    int i;
    GMT_LONG npoints;

    double proj_radius();

    double x[1000], y[1000];
    double radius_size;
    double str, radius, increment;
    double si, co;

    int no_fill[3], lineout = 1;

    no_fill[0] = -1;
    no_fill[1] = -1;
    no_fill[2] = -1;


/* compute radius size of the bubble */
    radius_size = size * 0.5;

/* outline the bubble */
    ps_plot(x0 + radius_size, y0, 3); 
/*    ps_circonf(x0, y0, radius_size); */
/*  argument is DIAMETER!!*/
    ps_circle(x0, y0, radius_size*2., no_fill, lineout);

        i = -1;
        increment = 1.;
        str = meca.NP1.str;
        while (str <= meca.NP1.str + 180. + EPSIL) {
            i++;
            radius = proj_radius(meca.NP1.str, meca.NP1.dip, str) * radius_size;   
            sincosd (str, &si, &co);
            x[i] = x0 + radius * si;
            y[i] = y0 + radius * co;
            str += increment;
        }
        npoints = i + 1;
        ps_line(x, y, npoints, 1, FALSE, FALSE);

        i = -1;
        increment = 1.;
        str = meca.NP2.str;
        while (str <= meca.NP2.str + 180. + EPSIL) {
            i++;
            radius = proj_radius(meca.NP2.str, meca.NP2.dip, str) * radius_size;
            sincosd (str, &si, &co);
            x[i] = x0 + radius * si;
            y[i] = y0 + radius * co;
            str += increment;
        }
        npoints = i + 1;
        ps_line(x, y, npoints, 1, FALSE, FALSE);
        return(radius_size*2.);
}

/*********************************************************************/
double ps_plan(double x0,double y0,st_me meca,double size,int num_of_plane)

/* Genevieve Patau */

{

	int i;
	GMT_LONG npoints;

    double proj_radius();

    double x[1000], y[1000];
    double radius_size;
    double str, radius, increment;
    double si, co;

    int rgb[3], lineout=1;

    rgb[0] = -1;
    rgb[1] = -1;
    rgb[2] = -1;


/* compute radius size of the bubble */
    radius_size = size * 0.5;

/* outline the bubble */
    ps_plot(x0 + radius_size, y0, 3);
/*  argument is DIAMETER!!*/
    ps_circle(x0, y0, radius_size*2., rgb, lineout);

      switch (num_of_plane) {
        case 1:
               i = -1;
               increment = 1.;
               str = meca.NP1.str;
               while (str <= meca.NP1.str + 180. + EPSIL) {
                   i++;
                   radius = proj_radius(meca.NP1.str, meca.NP1.dip, str) * radius_size;
                   sincosd (str, &si, &co);
                   x[i] = x0 + radius * si;
                   y[i] = y0 + radius * co;
                   str += increment;
               }
               npoints = i + 1;
               ps_line(x, y, npoints, 1, FALSE, FALSE);
               break;

        case 2:
               i = -1;
               increment = 1.;
               str = meca.NP2.str;
               while (str <= meca.NP2.str + 180. + EPSIL) {
                   i++;
                   radius = proj_radius(meca.NP2.str, meca.NP2.dip, str) * radius_size;
                   sincosd (str, &si, &co);
                   x[i] = x0 + radius * si;
                   y[i] = y0 + radius * co;
                   str += increment;
               }
               npoints = i + 1;
               ps_line(x, y, npoints, 1, FALSE, FALSE);
               break;
      }
      return(radius_size*2.);
}

/*********************************************************************/
double zero_360(double str)

  /* put an angle between 0 and 360 degrees */

  /* Genevieve Patau */

{
    if (str >= 360.)
        str -= 360.;
    else if (str < 0.)
        str += 360.;
    return(str);
}

/**********************************************************************/
double computed_mw(struct MOMENT moment,double ms)

  /*
     Compute mw-magnitude from seismic moment or MS magnitude.
 */

  /* Genevieve Patau from
Thorne Lay, Terry C. Wallace
Modern Global Seismology
Academic Press
p. 384
 */

{
    double mw;

    if (moment.exponent == 0)
        mw = ms;
    else
        mw = (log10(moment.mant) + (double)moment.exponent - 16.1) * 2. / 3.;

    return(mw);
}

/*********************************************************************/
double datan2(double y,double x)
 
  /*  compute arctg in degrees, between -180 et +180. */

  /* Genevieve Patau */

{
    double arctg;
    double rdeg = 180. / 3.14159265;
  
    if (fabs(x) < EPSIL) {
        if (fabs(y) < EPSIL) {
/*
            fprintf(stderr, "undetermined form 0. / 0.");
            exit(EXIT_FAILURE);
*/
            arctg = 0.;
        }
        else
            arctg = y < 0. ? -90. : 90.;
    }
    else if (x < 0.)
        arctg = y < 0. ? atan(y / x) * rdeg - 180. : atan(y / x) * rdeg + 180.;
    else 
        arctg = atan(y / x) * rdeg;

    return(arctg);
}

/*********************************************************************/
double computed_strike1(struct nodal_plane NP1)

/*
   Compute the strike of the decond nodal plane when are given
   strike, dip and rake for the first nodal plane with AKI & RICHARD's
   convention.
   Angles are in degrees.
*/

/* Genevieve Patau */

{
    double str2;
    double cd1 = cosd(NP1.dip);
    double temp;
    double cp2, sp2;
    double am = (GMT_IS_ZERO (NP1.rake) ? 1. : NP1.rake /fabs(NP1.rake)); 
    double ss, cs, sr, cr;

    sincosd (NP1.rake, &sr, &cr);
    sincosd (NP1.str, &ss, &cs);
    if (cd1 < EPSIL && fabs(cr) < EPSIL) {
/*
        fprintf(stderr, "\nThe second plane is horizontal;");
        fprintf(stderr, "\nStrike is undetermined.");
        fprintf(stderr, "\nstr2 = NP1.str + 180. is taken to define");
        fprintf(stderr, "\nrake in the second plane.");
*/
        str2 = NP1.str + 180.;
    }
    else {
        temp = cr * cs;
        temp += sr * ss * cd1;
        sp2 = -am * temp;
        temp = ss * cr;
        temp -= sr *  cs * cd1;
        cp2 = am * temp;
        str2 = datan2(sp2, cp2);
        str2 = zero_360(str2);
    }
    return(str2);
}

/*********************************************************************/
double computed_dip1(struct nodal_plane NP1)

/*
   Compute second nodal plane dip when are given strike,
   dip and rake for the first nodal plane with AKI & RICHARD's
   convention.
   Angles are in degrees.
*/

/* Genevieve Patau */

{
    double am = (GMT_IS_ZERO (NP1.rake) ? 1. : NP1.rake / fabs(NP1.rake));
    double dip2;
  
    dip2 = acos(am * sind(NP1.rake) * sind(NP1.dip)) / D2R;

    return(dip2);
}

/*********************************************************************/
double computed_rake1(struct nodal_plane NP1)

/* 
   Compute rake in the second nodal plane when strike ,dip
   and rake are given for the first nodal plane with AKI & 
   RICHARD's convention.
   Angles are in degrees.
*/

/* Genevieve Patau */

{
    double computed_strike1();
    double computed_dip1();
  
    double rake2, sinrake2;
    double str2 = computed_strike1(NP1);
    double dip2 = computed_dip1(NP1);
    double am = (GMT_IS_ZERO (NP1.rake) ? 1. : NP1.rake / fabs(NP1.rake));
    double sd, cd, ss, cs;
    sincosd (NP1.dip, &sd, &cd);
    sincosd (NP1.str - str2, &ss, &cs);

    if (fabs(dip2 - 90.) < EPSIL)
        sinrake2 = am * cd;
    else
        sinrake2 = -am * sd * cs / cd;

    rake2 = datan2(sinrake2, -am * sd * ss);

    return(rake2);
}

/*********************************************************************/
double computed_dip2(double str1,double dip1,double str2)

/* 
   Compute second nodal plane dip when are given
   strike and dip for the first plane and strike for 
   the second plane.
   Angles are in degrees.
   Warning : if dip1 == 90 and cos(str1 - str2) == 0
             the second plane dip is undetermined
             and the only first plane will be plotted.
*/
 
/* Genevieve Patau */

{
    double dip2;
    double cosdp12 = cosd(str1 - str2);

    if (fabs(dip1 - 90.) < EPSIL && fabs(cosdp12) < EPSIL) {
            dip2 = 1000.; /* (only first plane will be plotted) */
    }
    else {
        dip2 = datan2(cosd(dip1), -sind(dip1) * cosdp12);
    }

    return(dip2);
}

/*********************************************************************/
double computed_rake2(double str1,double dip1,double str2,double dip2,double fault)

/* 
   Compute rake in the second nodal plane when strike and dip
   for first and second nodal plane are given with a double
   characterizing the fault :
                          +1. inverse fault
                          -1. normal fault.
   Angles are in degrees.
*/

/* Genevieve Patau */

{
    double rake2, sinrake2;
    double sd, cd, ss, cs;

    sincosd (str1 - str2, &ss, &cs);

    sd = sind(dip1);        cd = cosd(dip2);
    if (fabs(dip2 - 90.) < EPSIL)
        sinrake2 = fault * cd;
    else
        sinrake2 = -fault * sd * cs / cd;

    rake2 = datan2(sinrake2, - fault * sd * ss);

    return(rake2);
}

/*********************************************************************/
void define_second_plane(struct nodal_plane NP1,struct nodal_plane *NP2)

/*
    Compute strike, dip, slip for the second nodal plane
    when are given strike, dip and rake for the first one.
*/
/*  Genevieve Patau */

{

    NP2->str = computed_strike1(NP1);
    NP2->dip = computed_dip1(NP1);
    NP2->rake  = computed_rake1(NP1);
}

/*********************************************************************/
double null_axis_dip(double str1,double dip1,double str2,double dip2)

/* 
   compute null axis dip when strike and dip are given
   for each nodal plane.
   Angles are in degrees.
*/

/* Genevieve Patau */

{
    double den;
  
    den = asin(sind(dip1) * sind(dip2) * sind(str1 - str2)) / D2R; 
    if (den < 0.)
        den = -den;
    return(den);
}

/*********************************************************************/
double null_axis_strike(double str1,double dip1,double str2,double dip2)

/*
   Compute null axis strike when strike and dip are given
   for each nodal plane.
   Angles are in degrees.
*/

/* Genevieve Patau */

{
    double phn, cosphn, sinphn;
    double sd1, cd1, sd2, cd2, ss1, cs1, ss2, cs2;
    
    sincosd (dip1, &sd1, &cd1);
    sincosd (dip2, &sd2, &cd2);
    sincosd (str1, &ss1, &cs1);
    sincosd (str2, &ss2, &cs2);

    cosphn = sd1 * cs1 * cd2 - sd2 * cs2 * cd1;
    sinphn = sd1 * ss1 * cd2 - sd2 * ss2 * cd1;
    if (sind(str1 - str2) < 0.) {
        cosphn = -cosphn;
        sinphn = -sinphn;
    } 
    phn = datan2(sinphn, cosphn);
    if (phn < 0.)
        phn += 360.;
    return(phn);
}

/*********************************************************************/
double proj_radius(double str1,double dip1,double str)

/*
   Compute the vector radius for a given strike,
   equal area projection, inferior sphere.
   Strike and dip of the plane are given.
*/

/* Genevieve Patau */

{
    double dip, r;
 
    if (fabs(dip1 - 90.) < EPSIL) {
/*
        printf("\nVertical plane : strike is constant.");
        printf("\nFor ps_mechanism r == 1 for str = str1");
        printf("\n            else r == 0. is used.");
*/
        r = (fabs(str - str1) < EPSIL || fabs(str - str1 - 180) < EPSIL) ? 1. : 0.;
    }
    else {
        dip = atan(tand(dip1) * sind(str - str1));
        r = sqrt(2.) * sin(M_PI_4 - dip / 2.);
    }
    return(r);
}

/*********************************************************************/
/*
Compute p-T axis strikes and dips from seismic moment tensor components.
Input moment tensor components mrr mtt mff mrt mrf mtf.

Angles are in degrees.
value, azimuth (en degres), plunge (in degrees) for T, N, P axis.

Genevieve Patau, 18 mars 1999
*/

#define NP 3
#define NMAT 1
#define NRANSI
#define ROTATE(a,i,j,k,l) g=a[i][j];h=a[k][l];a[i][j]=g-s*(h+g*tau);\
        a[k][l]=h+s*(g-h*tau);

void jacobi(float **a, GMT_LONG n, float d[], float **v, int *nrot)
{
        GMT_LONG j,iq,ip,i;
        float tresh,theta,tau,t,sm,s,h,g,c,*b,*z;

        b=vector((size_t)1,n);
        z=vector((size_t)1,n);
        for (ip=1;ip<=n;ip++) {
                for (iq=1;iq<=n;iq++) v[ip][iq]=0.0;
                v[ip][ip]=1.0;
        }
        for (ip=1;ip<=n;ip++) {
                b[ip]=d[ip]=a[ip][ip];
                z[ip]=0.0;
        }
        *nrot=0;
        for (i=1;i<=50;i++) {
                sm=0.0;
                for (ip=1;ip<=n-1;ip++) {
                        for (iq=ip+1;iq<=n;iq++)
                                sm += (float)fabs((double)a[ip][iq]);
                }
                if (sm == 0.0) {
                        free_vector(z,(size_t)1,n);
                        free_vector(b,(size_t)1,n);
                        return;
                }
                if (i < 4)
                        tresh=(float)0.2*sm/((float)(n*n));
                else
                        tresh=0.0;
                for (ip=1;ip<=n-1;ip++) {
                        for (iq=ip+1;iq<=n;iq++) {
                                g=(float)(100.0*fabs(a[ip][iq]));
                                if (i > 4 && (float)(fabs(d[ip])+g) == (float)fabs(d[ip])
                                        && (float)(fabs(d[iq])+g) == (float)fabs(d[iq]))
                                        a[ip][iq]=0.0;
                                else if (fabs(a[ip][iq]) > tresh) {
                                        h=d[iq]-d[ip];
                                        if ((float)(fabs(h)+g) == (float)fabs(h))
                                                t=(a[ip][iq])/h;
                                        else {
                                                theta=(float)0.5*h/(a[ip][iq]);
                                                t=(float)(1.0/(fabs(theta)+sqrt(1.0+theta*theta)));
                                                if (theta < 0.0) t = -t;
                                        }
                                        c=(float)(1.0/sqrt(1+t*t));
                                        s=t*c;
                                        tau=s/((float)1.0+c);
                                        h=t*a[ip][iq];
                                        z[ip] -= h;
                                        z[iq] += h;
                                        d[ip] -= h;
                                        d[iq] += h;
                                        a[ip][iq]=0.0;
                                        for (j=1;j<=ip-1;j++) {
                                                ROTATE(a,j,ip,j,iq)
                                        }
                                        for (j=ip+1;j<=iq-1;j++) {
                                                ROTATE(a,ip,j,j,iq)
                                        }
                                        for (j=iq+1;j<=n;j++) {
                                                ROTATE(a,ip,j,iq,j)
                                        }
                                        for (j=1;j<=n;j++) {
                                                ROTATE(v,j,ip,j,iq)
                                        }
                                        ++(*nrot);
                                }
                        }
                }
                for (ip=1;ip<=n;ip++) {
                        b[ip] += z[ip];
                        d[ip]=b[ip];
                        z[ip]=0.0;
                }
        }
        nrerror("Too many iterations in routine jacobi");
}

#undef ROTATE
#undef NRANSI
/* (C) Copr. 1986-92 Numerical Recipes Software W".. */

void momten2axe(struct M_TENSOR mt,struct AXIS *T,struct AXIS *N,struct AXIS *P)

{
/*
 * routine jacobi from Numerical Recipes is used.
 * W.H. Press, S.A. Teukolsky, W.T. Vetterling, B.P. Flannery
 * Numerical Recipes in C
 * Cambridge University press
 */
    int j,kk,nrot;
    int jj[3];
    float a[3][3];
    float *d,*r,**v,**e;
    float val[3], azi[3], plu[3];
    static int num=3;

    float min,max,mid;
    float az[3], pl[3];

    a[0][0]=(float)mt.f[0]; a[0][1]=(float)mt.f[3]; a[0][2]=(float)mt.f[4];
    a[1][0]=(float)mt.f[3]; a[1][1]=(float)mt.f[1]; a[1][2]=(float)mt.f[5];
    a[2][0]=(float)mt.f[4]; a[2][1]=(float)mt.f[5]; a[2][2]=(float)mt.f[2];

    d=vector((GMT_LONG)1,(GMT_LONG)NP);
    r=vector((GMT_LONG)1,(GMT_LONG)NP);
    v=matrix((GMT_LONG)1,(GMT_LONG)NP,(GMT_LONG)1,(GMT_LONG)NP);
    e=convert_matrix(&a[0][0],(GMT_LONG)1,(GMT_LONG)num,(GMT_LONG)1,(GMT_LONG)num);
    jacobi(e,(GMT_LONG)num,d,v,&nrot);

/* sort eigenvalues */
    max = -10000.;
    for (j=1;j<=num;j++) if (max<=d[j]) {max=d[j];jj[0]=j;}
    min = 10000.;
    for (j=1;j<=num;j++) if (min>=d[j]) {min=d[j];jj[2]=j;}
    mid = 0.;
    for (j=1;j<=num;j++) if (j!=jj[0]&&j!=jj[2]) jj[1]=j;

    for (j=1;j<=num;j++) {
        kk=jj[j-1];
        pl[kk]=(float)(asin(- v[1][kk]));
        az[kk]=(float)(atan2(v[3][kk],- v[2][kk]));
        if (pl[kk]<=0.) {pl[kk]=-pl[kk]; az[kk]+=(float)(M_PI);}
        if (az[kk]<0.) az[kk]+=(float)(2.*M_PI);
        else if (az[kk]>(float)(2.*M_PI)) az[kk]-=(float)(2.*M_PI);
        pl[kk]*=(float)(180./M_PI);
        az[kk]*=(float)(180./M_PI);
        val[j-1] = d[kk]; azi[j-1] = az[kk]; plu[j-1] = pl[kk];
    }
    T->val = (double)val[0]; T->e = mt.expo; T->str = (double)azi[0]; T->dip = (double)plu[0];
    N->val = (double)val[1]; N->e = mt.expo; N->str = (double)azi[1]; N->dip = (double)plu[1];
    P->val = (double)val[2]; P->e = mt.expo; P->str = (double)azi[2]; P->dip = (double)plu[2];
}

/***************************************************************************************/
double ps_tensor(double x0,double y0,double size,struct AXIS T,struct AXIS N,struct AXIS P,int c_rgb[3],int e_rgb[3], int outline, int plot_zerotrace)
{
    int d, b = 1, m;
    int i, ii, n = 0, j = 1, j2 = 0, j3 = 0;
    GMT_LONG npoints;
    int lineout = 1;
    int rgb1[3], rgb2[3];
    int big_iso = 0;

    double a[3], p[3], v[3];
    double vi, iso, f;
    double fir, s2alphan, alphan;
    double cfi, sfi, can, san;
    double cpd, spd, cpb, spb, cpm, spm;
    double cad, sad, cab, sab, cam, sam;
    double xz, xn, xe;
    double az = 0., azp = 0., takeoff, r;
    double azi[3][2];
    double x[400], y[400], x2[400], y2[400], x3[400], y3[400];
    double xp1[800], yp1[800], xp2[400], yp2[400];
    double radius_size;
    double si, co;

    a[0] = T.str; a[1] = N.str; a[2] = P.str;
    p[0] = T.dip; p[1] = N.dip; p[2] = P.dip;
    v[0] = T.val; v[1] = N.val; v[2] = P.val;

    vi = (v[0] + v[1] + v[2]) / 3.;
    for (i=0; i<=2; i++) v[i] = v[i] - vi;

    radius_size = size * 0.5;

    if (fabs(squared(v[0]) + squared(v[1]) + squared(v[2])) < EPSIL) {
        /* pure implosion-explosion */
        if (vi > 0.) {
            ps_circle(x0, y0, radius_size*2., c_rgb, lineout);
        }
        if (vi < 0.) {
            ps_circle(x0, y0, radius_size*2., e_rgb, lineout);
        }
        return(radius_size*2.);
    }

    if (fabs(v[0]) >= fabs(v[2])) {
        d = 0;
        m = 2;
    }
    else {
        d = 2;
        m = 0;
    }

    if (plot_zerotrace) vi = 0.;

    f = - v[1] / v[d];
    iso = vi / v[d];

/* Cliff Frohlich, Seismological Research letters,
 * Vol 7, Number 1, January-February, 1996
 * Unless the isotropic parameter lies in the range
 * between -1 and 1 - f there will be no nodes whatsoever */

    if (iso < -1) {
        ps_circle(x0, y0, radius_size*2., e_rgb, lineout);
        return(radius_size*2.);
    }
    else if (iso > 1-f) {
        ps_circle(x0, y0, radius_size*2., c_rgb, lineout);
        return(radius_size*2.);
    }

    sincosd (p[d], &spd, &cpd);
    sincosd (p[b], &spb, &cpb);
    sincosd (p[m], &spm, &cpm);
    sincosd (a[d], &sad, &cad);
    sincosd (a[b], &sab, &cab);
    sincosd (a[m], &sam, &cam);

    for (i=0; i<360; i++) {
        fir = (double) i * D2R;
        s2alphan = (2. + 2. * iso) / (3. + (1. - 2. * f) * cos(2. * fir));
        if (s2alphan > 1.) big_iso++;
        else {
            alphan = asin(sqrt(s2alphan));
            sincos (fir, &sfi, &cfi);
            sincos (alphan, &san, &can);

            xz = can * spd + san * sfi * spb + san * cfi * spm;
            xn = can * cpd * cad + san * sfi * cpb * cab + san * cfi * cpm * cam;
            xe = can * cpd * sad + san * sfi * cpb * sab + san * cfi * cpm * sam;

            if (fabs(xn) < EPSIL && fabs(xe) < EPSIL) {
                takeoff = 0.;
                az = 0.;
            }
            else {
                az = atan2(xe, xn);
                if (az < 0.) az += M_PI * 2.;
                takeoff = acos(xz / sqrt(xz * xz + xn * xn + xe * xe));
            }
            if (takeoff > M_PI_2) {
                takeoff = M_PI - takeoff;
                az += M_PI;
                if (az > M_PI * 2.) az -= M_PI * 2.;
            }
            r = M_SQRT2 * sin(takeoff / 2.);
            sincos (az, &si, &co);
            if (i == 0) {
                azi[i][0] = az;
                x[i] = x0 + radius_size * r * si;
                y[i] = y0 + radius_size * r * co;
                azp = az;
            }
            else {
                if (fabs(fabs(az - azp) - M_PI) < D2R * 10.) {
                        azi[n][1] = azp;
                        azi[++n][0] = az;
                }
                if (fabs(fabs(az -azp) - M_PI * 2.) < D2R * 2.) {
                        if (azp < az) azi[n][0] += M_PI * 2.;
                        else azi[n][0] -= M_PI * 2.;
                }
                switch (n) {
                        case 0 :
                                x[j] = x0 + radius_size * r * si;
                                y[j] = y0 + radius_size * r * co;
                                j++;
                                break;
                        case 1 :
                                x2[j2] = x0 + radius_size * r * si;
                                y2[j2] = y0 + radius_size * r * co;
                                j2++;
                                break;
                        case 2 :
                                x3[j3] = x0 + radius_size * r * si;
                                y3[j3] = y0 + radius_size * r * co;
                                j3++;
                                break;
                }
                azp = az;
            }
        }
    }
    azi[n][1] = az;

    if (v[1] < 0.) for (i=0;i<=2;i++) {rgb1[i] = c_rgb[i]; rgb2[i] = e_rgb[i];}
    else for (i=0;i<=2;i++) {rgb1[i] = e_rgb[i]; rgb2[i] = c_rgb[i];}

    ps_circle(x0, y0, radius_size*2., rgb2, lineout);
    switch(n) {
        case 0 :
            for (i=0; i<360; i++) {
                xp1[i] = x[i]; yp1[i] = y[i];
            }
            npoints = i;
            ps_polygon(xp1, yp1, npoints, rgb1, outline);
            break;
        case 1 :
            for (i=0; i<j; i++) {
                xp1[i] = x[i]; yp1[i] = y[i];
            }
            if (azi[0][0] - azi[0][1] > M_PI) azi[0][0] -= M_PI * 2.;
            else if (azi[0][1] - azi[0][0] > M_PI) azi[0][0] += M_PI * 2.;
            if (azi[0][0] < azi[0][1])
                for (az = azi[0][1] - D2R; az > azi[0][0]; az -= D2R) {
                    sincos (az, &si, &co);
                    xp1[i] = x0 + radius_size * si;
                    yp1[i++] = y0 + radius_size * co;
                }
            else
                for (az = azi[0][1] + D2R; az < azi[0][0]; az += D2R) {
                    sincos (az, &si, &co);
                    xp1[i] = x0 + radius_size * si; 
                    yp1[i++] = y0 + radius_size * co;
                }
            npoints = i;
            ps_polygon(xp1, yp1, npoints, rgb1, outline);
            for (i=0; i<j2; i++) {
                xp2[i] = x2[i]; yp2[i] = y2[i];
            }
            if (azi[1][0] - azi[1][1] > M_PI) azi[1][0] -= M_PI * 2.;
            else if (azi[1][1] - azi[1][0] > M_PI) azi[1][0] += M_PI * 2.;
            if (azi[1][0] < azi[1][1])
                for (az = azi[1][1] - D2R; az > azi[1][0]; az -= D2R) {
                    sincos (az, &si, &co);
                    xp2[i] = x0 + radius_size * si; 
                    yp2[i++] = y0 + radius_size * co;
                }
            else
                for (az = azi[1][1] + D2R; az < azi[1][0]; az += D2R) {
                    sincos (az, &si, &co);
                    xp2[i] = x0 + radius_size * si; 
                    yp2[i++] = y0 + radius_size * co;
                }
            npoints = i;
            ps_polygon(xp2, yp2, npoints, rgb1, outline);
            break;
        case 2 :
            for (i=0; i<j3; i++) {
                xp1[i] = x3[i]; yp1[i] = y3[i];
            }
            for (ii=0; ii<j; ii++) {
                xp1[i] = x[ii]; yp1[i++] = y[ii];
            }

            if (big_iso) {
                for (ii=j2-1; ii>=0; ii--) {
                    xp1[i] = x2[ii]; yp1[i++] = y2[ii];
                }
                npoints = i;
                ps_polygon(xp1, yp1, npoints, rgb1, outline);
                break;
            }

            if (azi[2][0] - azi[0][1] > M_PI) azi[2][0] -= M_PI * 2.;
            else if (azi[0][1] - azi[2][0] > M_PI) azi[2][0] += M_PI * 2.;
            if (azi[2][0] < azi[0][1])
                for (az = azi[0][1] - D2R; az > azi[2][0]; az -= D2R) {
                    sincos (az, &si, &co);
                    xp1[i] = x0+ radius_size * si; 
                    yp1[i++] = y0+ radius_size * co;
                }
            else
                for (az = azi[0][1] + D2R; az < azi[2][0]; az += D2R) {
                    sincos (az, &si, &co);
                    xp1[i] = x0+ radius_size * si; 
                    yp1[i++] = y0+ radius_size * co;
                }
            npoints = i;
            ps_polygon(xp1, yp1, npoints, rgb1, outline);
            for (i=0; i<j2; i++) {
                xp2[i] = x2[i]; yp2[i] = y2[i];
            }
            if (azi[1][0] - azi[1][1] > M_PI) azi[1][0] -= M_PI * 2.;
            else if (azi[1][1] - azi[1][0] > M_PI) azi[1][0] += M_PI * 2.;
            if (azi[1][0] < azi[1][1])
                for (az = azi[1][1] - D2R; az > azi[1][0]; az -= D2R) {
                    sincos (az, &si, &co);
                    xp2[i] = x0+ radius_size * si; 
                    yp2[i++] = y0+ radius_size * co;
                }
            else
                for (az = azi[1][1] + D2R; az < azi[1][0]; az += D2R) {
                    sincos (az, &si, &co);
                    xp2[i] = x0+ radius_size * si; 
                    yp2[i++] = y0+ radius_size * co;
                }
            npoints = i;
            ps_polygon(xp2, yp2, npoints, rgb1, outline);
            break;
    }
    return(radius_size*2.);
}

/***************************************************************************************/
/*
Calculate double couple from principal axes.
Angles are in degrees.

Genevieve Patau, 16 juin 1997
*/

void axe2dc(struct AXIS T,struct AXIS P,struct nodal_plane *NP1,struct nodal_plane *NP2)

{

     double pp, dp, pt, dt;
     double p1, d1, p2, d2;
     double PII = M_PI * 2.;
     double cdp, sdp, cdt, sdt;
     double cpt, spt, cpp, spp;
     double amz, amy, amx;
     double im;

     pp = P.str * D2R; dp = P.dip * D2R;
     pt = T.str * D2R; dt = T.dip * D2R;

     sincos (dp, &sdp, &cdp);
     sincos (dt, &sdt, &cdt);
     sincos (pt, &spt, &cpt);
     sincos (pp, &spp, &cpp);

     cpt *= cdt; spt *= cdt;
     cpp *= cdp; spp *= cdp;

     amz = sdt + sdp; amx = spt + spp; amy = cpt + cpp;
     d1 = atan2(sqrt(amx*amx + amy*amy), amz);
     p1 = atan2(amy, -amx);
     if (d1 > M_PI_2) {
          d1 = M_PI - d1;
          p1 += M_PI;
          if (p1 > PII) p1 -= PII;
     }
     if (p1 < 0.) p1 += PII;

     amz = sdt - sdp; amx = spt - spp; amy = cpt - cpp;
     d2 = atan2(sqrt(amx*amx + amy*amy), amz);
     p2 = atan2(amy, -amx);
     if (d2 > M_PI_2) {
          d2 = M_PI - d2;
          p2 += M_PI;
          if (p2 > PII) p2 -= PII;
     }
     if (p2 < 0.) p2 += PII;

     NP1->dip = d1 / D2R; NP1->str = p1 / D2R;
     NP2->dip = d2 / D2R; NP2->str = p2 / D2R;

     im = 1;
     if (dp > dt) im = -1;
     NP1->rake = computed_rake2(NP2->str,NP2->dip,NP1->str,NP1->dip,im);
     NP2->rake = computed_rake2(NP1->str,NP1->dip,NP2->str,NP2->dip,im);
}

/*********************************************************************/
void ps_pt_axis(double x0,double y0,st_me meca,double size,double *pp,double *dp,double *pt,double *dt,double *xp,double *yp,double *xt,double *yt)

/*
From FORTRAN routines of Anne Deschamps :
compute azimuth and plungement of P-T axis
from nodal plane strikes, dips and rakes.
*/

{
        int im = 0;
        int pure_strike_slip = 0;
        double cd1, sd1, cd2, sd2;
        double cp1, sp1, cp2, sp2;
        double amz, amx, amy, dx, px, dy, py;
        double radius;

        if (fabs(sin(meca.NP1.rake * D2R)) > EPSIL) im = (int) (meca.NP1.rake / fabs(meca.NP1.rake));
        else if (fabs(sin(meca.NP2.rake * D2R)) > EPSIL) im = (int) (meca.NP2.rake / fabs(meca.NP2.rake));
        else pure_strike_slip = 1;

        size *= 0.5;

        if (pure_strike_slip) {
            if (cos(meca.NP1.rake * D2R) < 0.) {
                *pp = zero_360(meca.NP1.str + 45.);
                *pt = zero_360(meca.NP1.str - 45.);
            }
            else {
                *pp = zero_360(meca.NP1.str - 45.);
                *pt = zero_360(meca.NP1.str + 45.);
            }
            *dp = 0.;
            *dt = 0.;
            radius = 0.97;
            *xp = radius * sin(*pp * D2R) * size + x0;
            *yp = radius * cos(*pp * D2R) * size + y0;
            *xt = radius * sin(*pt * D2R) * size + x0;
            *yt = radius * cos(*pt * D2R) * size + y0;
        }
        else {
            cd1 = cos(meca.NP1.dip * D2R) *  M_SQRT2;
            sd1 = sin(meca.NP1.dip * D2R) *  M_SQRT2;
            cd2 = cos(meca.NP2.dip * D2R) *  M_SQRT2;
            sd2 = sin(meca.NP2.dip * D2R) *  M_SQRT2;
            cp1 = - cos(meca.NP1.str * D2R) *  sd1;
            sp1 = sin(meca.NP1.str * D2R) *  sd1;
            cp2 = - cos(meca.NP2.str * D2R) *  sd2;
            sp2 = sin(meca.NP2.str * D2R) *  sd2;

            amz = - (cd1 + cd2);
            amx = - (sp1 + sp2);
            amy = cp1 + cp2;
            dx = atan2(sqrt(amx * amx + amy * amy), amz) - M_PI_2;
            px = atan2(amy, - amx);
            if (px < 0.) px += TWO_PI;

            amz = cd1 - cd2;
            amx = sp1 - sp2;
            amy = - cp1 + cp2;
            dy = atan2(sqrt(amx * amx + amy * amy), - fabs(amz)) - M_PI_2;
            py = atan2(amy, - amx);
            if (amz > 0.) py -= M_PI;
            if (py < 0.) py += TWO_PI;

            if (im == 1) {
                    *dp = dy;
                    *pp = py;
                    *dt = dx;
                    *pt = px;
            }
            else {
                    *dp = dx;
                    *pp = px;
                    *dt = dy;
                    *pt = py;
            }
            radius = sqrt(1. - sin(*dp));
            if (radius >= 0.97) radius = 0.97;
            *xp = radius * sin(*pp) * size + x0;
            *yp = radius * cos(*pp) * size + y0;
            radius = sqrt(1. - sin(*dt));
            if (radius >= 0.97) radius = 0.97;
            *xt = radius * sin(*pt) * size + x0;
            *yt = radius * cos(*pt) * size + y0;
            *pp *= 180. / M_PI;
            *dp *= 180. / M_PI;
            *pt *= 180. / M_PI;
            *dt *= 180. / M_PI;
        }
}

/*********************************************************************/
void dc_to_axe(st_me meca,struct AXIS *T,struct AXIS *N,struct AXIS *P)

/*
From FORTRAN routines of Anne Deschamps :
compute azimuth and plungement of P-T axis
from nodal plane strikes, dips and rakes.
*/

{
        int im = 0;
        int pure_strike_slip = 0;
        double cd1, sd1, cd2, sd2;
        double cp1, sp1, cp2, sp2;
        double amz, amx, amy, dx, px, dy, py;

        if (fabs(sind(meca.NP1.rake)) > EPSIL) im = (int) (meca.NP1.rake / fabs(meca.NP1.rake));
        else if (fabs(sind(meca.NP2.rake)) > EPSIL) im = (int) (meca.NP2.rake / fabs(meca.NP2.rake));
        else pure_strike_slip = 1;

        if (pure_strike_slip) {
            if (cosd(meca.NP1.rake) < 0.) {
                P->str = zero_360(meca.NP1.str + 45.);
                T->str = zero_360(meca.NP1.str - 45.);
            }
            else {
                P->str = zero_360(meca.NP1.str - 45.);
                T->str = zero_360(meca.NP1.str + 45.);
            }
            P->dip = 0.;
            T->dip = 0.;
        }
        else {
            cd1 = cosd(meca.NP1.dip) *  M_SQRT2;
            sd1 = sind(meca.NP1.dip) *  M_SQRT2;
            cd2 = cosd(meca.NP2.dip) *  M_SQRT2;
            sd2 = sind(meca.NP2.dip) *  M_SQRT2;
            cp1 = - cosd(meca.NP1.str) * sd1;
            sp1 = sind(meca.NP1.str) * sd1;
            cp2 = - cosd(meca.NP2.str) * sd2;
            sp2 = sind(meca.NP2.str) * sd2;

            amz = - (cd1 + cd2);
            amx = - (sp1 + sp2);
            amy = cp1 + cp2;
            dx = atan2(sqrt(amx * amx + amy * amy), amz) - M_PI_2;
            px = atan2(amy, - amx);
            if (px < 0.) px += TWO_PI;

            amz = cd1 - cd2;
            amx = sp1 - sp2;
            amy = - cp1 + cp2;
            dy = atan2(sqrt(amx * amx + amy * amy), - fabs(amz)) - M_PI_2;
            py = atan2(amy, - amx);
            if (amz > 0.) py -= M_PI;
            if (py < 0.) py += TWO_PI;

            if (im == 1) {
                    P->dip = dy;
                    P->str = py;
                    T->dip = dx;
                    T->str = px;
            }
            else {
                    P->dip = dx;
                    P->str = px;
                    T->dip = dy;
                    T->str = py;
            }

        }

        T->str /= D2R;
        T->dip /= D2R;
        P->str /= D2R;
        P->dip /= D2R;

        N->str = null_axis_strike(T->str, T->dip, P->str, P->dip);
        N->dip = null_axis_dip(T->str, T->dip, P->str, P->dip);
}

/*********************************************************************/
void axis2xy(double x0,double y0,double size,double pp,double dp,double pt,double dt,double *xp,double *yp,double *xt,double *yt)
/* angles are in degrees */
{
            double radius;
            double spp, cpp, spt, cpt;

            sincosd (pp, &spp, &cpp);
            sincosd (pt, &spt, &cpt);

            size *= 0.5;
            radius = sqrt(1. - sind(dp));
            if (radius >= 0.97) radius = 0.97;
            *xp = radius * spp * size + x0;
            *yp = radius * cpp * size + y0;
            radius = sqrt(1. - sind(dt));
            if (radius >= 0.97) radius = 0.97;
            *xt = radius * spt * size + x0;
            *yt = radius * cpt * size + y0;
}
