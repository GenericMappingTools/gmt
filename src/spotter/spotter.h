/*--------------------------------------------------------------------
 *	$Id: spotter.h,v 1.1.1.1 2000-12-28 01:23:45 gmt Exp $
 *
 *   Copyright (c) 1999-2001 by P. Wessel
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   Contact info: www.soest.hawaii.edu/wessel
 *--------------------------------------------------------------------*/
/*
 * SPOTTER.H: Include file for programs that link with libspotter.a.
 *
 * Author:	Paul Wessel, SOEST, Univ. of Hawaii, Honolulu, HI, USA
 * Date:	02-MAR-2000
 * Version:	1.1
 *
 */

#include "gmt.h"	/* Requires GMT to compile and link */

#define SPOTTER_VERSION "1.0"
#define EQ_RAD 6371.0087714
#define KM_PR_DEG (EQ_RAD * M_PI / 180.0)
#define BIG_CHUNK 50000

struct EULER {	/* Structure with info on each Euler (stage) pole */
	double lon, lat;		/* Location of Euler pole in degrees */
	double lon_r, lat_r;		/* Location of Euler pole in radians */
	double t_start, t_stop;		/* Stage beginning and end time in Ma */
	double duration;		/* Stage duration in m.y. */
	double omega;			/* Rotation in Degrees/m.y. */
	double omega_r;			/* Rotation in Radians/m.y. */
	double sin_lat, cos_lat;	/* Sine and Cosine of pole latitude */
};

/* ANSI-C Function prototypes (see libspotter.c for details): */

EXTERN_MSC int spotter_init (char *file, struct EULER **p, int locus, int finite, double *t_max);
EXTERN_MSC int spotter_backtrack (double xp[], double yp[], double tp[], int np, struct EULER p[], int ns, double d_km, double t_zero, BOOLEAN do_time, double **c);
EXTERN_MSC int spotter_forthtrack (double xp[], double yp[], double tp[], int np, struct EULER p[], int ns, double d_km, double t_zero, BOOLEAN do_time, double **c);
