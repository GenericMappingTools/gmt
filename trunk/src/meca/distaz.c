/*	$Id$
 *    Copyright (c) 1996-2011 by G. Patau
 *    Distributed under the GNU Public Licence
 *    See README file for copying and redistribution conditions.
 */
 
#include "gmt.h"

#define CONSTANTE2 0.9931177
#define EPSILON 0.0001
#define RAYON 6371.0

void distaz (double lat1,double lon1,double lat2,double lon2,double *distrad,double *distdeg,double *distkm,double *az12rad,double *az12deg,double *az21rad,double *az21deg,GMT_LONG syscoord)
{
 /*
  Coordinates in degrees  : syscoord = 0
  Coordinates in radians : syscoord = 1
  Cartesian coordinates in km : syscoord = 2
*/

	double slat1, clat1, slon1, clon1, slat2, clat2, slon2, clon2;
	double a1, b1, g1, h1, a2, b2, g2, h2, c1, c3, c4, c5, c6;

	if (syscoord == 2) {
		*distkm = sqrt((lon1 - lon2) * (lon1 - lon2) + (lat1 - lat2) * (lat1 - lat2));
		*distrad = *distkm / RAYON;
		*distdeg = *distrad * R2D;
		*az12rad = atan2((lon2 - lon1), (lat2 - lat1));
		*az21rad = atan2((lon1 - lon2), (lat1 - lat2));
	}
	else {
		if (syscoord <= 0) {
			lat1 *= D2R;
			lon1 *= D2R;
			lat2 *= D2R;
			lon2 *= D2R;
			if ((M_PI_2 - fabs(lat1)) > EPSILON) lat1 = atan(CONSTANTE2 * tan(lat1));
			if((M_PI_2 - fabs(lat2)) > EPSILON) lat2 = atan(CONSTANTE2 * tan(lat2));
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
		g2 = slat2 * clon2;
		h2 = slat2 * slon2;

		c1 = a1 * a2 + b1 * b2 + slat1 * slat2;
		if (fabs(c1) < 0.94)
			*distrad = acos(c1);
		else if(c1 > 0.)
			*distrad = asin(sqrt((a1 - a2) * (a1 - a2) + (b1 - b2) * (b1 - b2) + (slat1 - slat2) * (slat1 - slat2)) / 2.) * 2.;
		else
			*distrad = acos(sqrt((a1 + a2) * (a1 + a2) + (b1 + b2) * (b1 + b2) + (slat1 + slat2) * (slat1 + slat2)) / 2.) * 2.;
		*distkm = *distrad * RAYON;
		*distdeg = *distrad * R2D;

		c3 = (a2 - slon1) * (a2 - slon1) + (b2 + clon1) * (b2 + clon1) + slat2 * slat2 - 2.;
		c4 = (a2 - g1) * (a2 - g1) + (b2 - h1) * (b2 - h1) + (slat2 + clat1) * (slat2 + clat1) - 2.;
		*az12rad = atan2(c3, c4);

		c5 = (a1 - slon2) * (a1 - slon2) + (b1 + clon2) * (b1 + clon2) + slat1 * slat1 - 2.;
		c6 = (a1 - g2) * (a1 - g2) + (b1 - h2) * (b1 - h2) + (slat1 + clat2) * (slat1 + clat2) - 2.;
		*az21rad = atan2(c5, c6);
	}

	if(*az12rad < 0.) *az12rad += M_PI * 2;
	*az12deg = *az12rad * R2D;
	if(*az21rad < 0.) *az21rad += M_PI * 2;
	*az21deg = *az21rad * R2D;

	return;
}
