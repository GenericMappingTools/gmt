/* $Id$
 *
 * Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 * See LICENSE.TXT file for copying and redistribution conditions.
 *
 * gmt_imgsubs.c -- subroutines supporting the GMT supplement imgsrc,
 * handling coordinate conversions for the "img" files created by
 * D T Sandwell and W H F Smith.
 * This is a complete rebuild of the old "imgsio" supplement, created
 * for GMT v. 3.1
 *
 * Further comments and definitions are in "gmt_imgsubs.h".
 *
 * Author:	W H F Smith
 * Date:	7 October 1998 
 *
 */


#include "gmt_imgsubs.h"

double  GMT_img_gud_fwd (double y)
{
	/* The Forward Gudermannian function.  Given y, the distance
	 * from the Equator to a latitude on a spherical Mercator map
	 * developed from a sphere of unit radius, returns the latitude
	 * in radians.  Should be called with -oo < y < +oo.  Returned
	 * value will be in -M_PI_2 < value < +M_PI_2.  */
	 
	return(2.0 * atan(exp(y)) - M_PI_2);
}

double  GMT_img_gud_inv (double phi)
{
	/* The Inverse Gudermannian function.  Given phi, a latitude
	 * in radians, returns the distance from the Equator to this
	 * latitude on a Mercator map tangent to a sphere of unit
	 * radius.  Should be called with -M_PI_2 < phi < +M_PI_2.
	 * Returned value will be in -oo < value < +oo.   */
	
	return(log(tan(M_PI_4 + 0.5 * phi)));
}

double  GMT_img_lat_to_ypix (double lat, struct GMT_IMG_COORD *coord)
{
	/* Given Latitude in degrees and pointer to coordinate struct,
	 * return (double) coordinate from top edge of input img file
	 * measured downward in coordinate pixels.  */
	 
	 return(coord->nytop - coord->radius * GMT_img_gud_inv(lat*D2R));
}

double  GMT_img_ypix_to_lat (double ypix, struct GMT_IMG_COORD *coord)
{
	/* Given Y coordinate, measured downward from top edge of 
	 * input img file in pixels, and pointer to coordinate struct,
	 * return Latitude in degrees.  */
	
	return( R2D * GMT_img_gud_fwd( (coord->nytop - ypix) / coord->radius) );
}

GMT_LONG GMT_img_setup_coord (struct GMT_CTRL *GMT, struct GMT_IMG_RANGE *r, struct GMT_IMG_COORD *c)
{
	/* Given the RANGE info, set up the COORD values.  Return (-1) on failure;
	 * 0 on success.  */

	if (r->maxlon < 360.0) {
		fprintf (GMT->session.std[GMT_ERR], "ERROR from GMT_img_setup_coord: Cannot handle maxlon < 360.\n");
		return (-1);
	}
	
	c->nxcol  = lrint (r->maxlon * 60.0 / r->mpixel);
	c->nx360  = lrint (360.0 * 60.0 / r->mpixel);
	c->radius = c->nx360 / (2.0 * M_PI);
	c->nytop  = lrint (c->radius * GMT_img_gud_inv(r->maxlat*D2R) );
	c->nyrow  = c->nytop - lrint (c->radius * GMT_img_gud_inv(r->minlat*D2R) );
	
	return (0);
}
