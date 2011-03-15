/* $Id: gmt_imgsubs.h,v 1.13 2011-03-15 02:06:37 guru Exp $
 *
 * Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 * See LICENSE.TXT file for copying and redistribution conditions.
 *
 * gmt_imgsubs.h -- header file supporting the GMT supplement imgsrc,
 * handling coordinate conversions for the "img" files created by
 * D T Sandwell and W H F Smith.
 * This is a complete rebuild of the old "imgsio" supplement, created
 * for GMT v. 3.1
 *
 * In addition to previous functionality, had to anticipate new
 * resolutions in future and wanted to preserve complete backward
 * compatibility to ancient versions which spanned 390 instead of
 * 360 degrees, etc.
 *
 * Main new functionality is option to average N by N square of 
 * input pixel information to create one output information.
 *
 * Author:  W H F Smith
 * Date:    7 October 1998 
 *
 */

#include "gmt.h"

/* The following values are used to initialize the default values
	controlling the range covered by the img file and the size 
	of a pixel in minutes of longitude.  The values shown here
	are for the 2-minute files currently in use (world_grav 7.2
	and topo_polish 6.2).  */

#define GMT_IMG_MAXLON		360.0
#define GMT_IMG_MINLAT		-72.0059773539
#define GMT_IMG_MAXLAT		+72.0059773539
#define GMT_IMG_MINLAT_80	-80.7380086280
#define GMT_IMG_MAXLAT_80	+80.7380086280

#define GMT_IMG_MPIXEL 2.0

/* The following structure contains info corresponding to 
 * the above values, which may be altered by the user at 
 * run time via argv[][] switches:  */

struct GMT_IMG_RANGE {
	double	maxlon;
	double	minlat;
	double	maxlat;
	double	mpixel;

};

/* The following structure contains info used to set up the 
 * coordinate transformations.  These are to be determined
 * based on the values in GMT_IMG_RANGE after argv[][] has
 * been parsed.  Two different structures will be used, one
 * representing the input file, and the other the output,
 * to simplify the computation of coordinates for N by N
 * averages in the output:  */
 
 struct GMT_IMG_COORD {
	double	radius;		/* # of pixels in 1 radian of longitude */
	GMT_LONG	nx360;		/* # of pixels in 360 degrees of longtd */
	GMT_LONG	nxcol;		/* # of columns in input img file  */
	GMT_LONG	nyrow;		/* # of rows in input img file  */
	GMT_LONG	nytop;		/* # of rows from Equator to top edge */
};

double	GMT_img_gud_fwd (double y);		/* Forward Gudermannian function */
double	GMT_img_gud_inv (double phi);	/* Inverse Gudermannian function */

/* Function returning Y coordinate, given Latitude and coordinate system. */
double	GMT_img_lat_to_ypix (double lat, struct GMT_IMG_COORD *coord);

/* Function returning Latitude, given Y coordinate and coordinate system. */
double	GMT_img_ypix_to_lat (double ypix, struct GMT_IMG_COORD *coord);

/* Function to set up the GMT_IMG_COORD based on GMT_IMG_RANGE */
GMT_LONG     GMT_img_setup_coord (struct GMT_CTRL *GMT, struct GMT_IMG_RANGE *r, struct GMT_IMG_COORD *c);
