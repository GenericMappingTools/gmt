/*--------------------------------------------------------------------
 *	$Id: spotter.h,v 1.8 2004-02-02 18:16:55 pwessel Exp $
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

#define SPOTTER_VERSION "1.1"
#define EQ_RAD 6371.0087714
#define KM_PR_DEG (EQ_RAD * M_PI / 180.0)
#define BIG_CHUNK 50000
#define T_2_PA	250.0
#define PA_2_T  (1.0 / T_2_PA)

struct EULER {	/* Structure with info on each Euler (stage) pole */
	double lon, lat;		/* Location of Euler pole in degrees */
	double lon_r, lat_r;		/* Location of Euler pole in radians */
	double t_start, t_stop;		/* Stage beginning and end time in Ma */
	double duration;		/* Stage duration in m.y. */
	double omega;			/* Rotation in Degrees/m.y. */
	double omega_r;			/* Rotation in Radians/m.y. */
	double sin_lat, cos_lat;	/* Sine and Cosine of pole latitude */
};

struct FLOWLINE {			/* Structure with the nearest nodes for a single flowline */
	int n;				/* number of points in this flowline */
	int ij;				/* Node in bathymetry grid where this flowline originated */
	int *node;			/* Nodes in CVA grid covered by this flowline */
	unsigned short *PA;		/* Predicted Ages along flowline (t = PI/250, to nearest 0.004 My) */
};

/* ANSI-C Function prototypes (see libspotter.c for details): */

EXTERN_MSC int spotter_init (char *file, struct EULER **p, int flowline, BOOLEAN finite_in, BOOLEAN finite_out, double *t_max, BOOLEAN verbose);
EXTERN_MSC int spotter_backtrack  (double xp[], double yp[], double tp[], int np, struct EULER p[], int ns, double d_km, double t_zero, BOOLEAN do_time, double wesn[], double **c);
EXTERN_MSC int spotter_forthtrack (double xp[], double yp[], double tp[], int np, struct EULER p[], int ns, double d_km, double t_zero, BOOLEAN do_time, double wesn[], double **c);
EXTERN_MSC void spotter_finite_to_stages (struct EULER p[], int n, BOOLEAN finite_rates, BOOLEAN stage_rates);
EXTERN_MSC void spotter_stages_to_finite (struct EULER p[], int n, BOOLEAN finite_rates, BOOLEAN stage_rates);
EXTERN_MSC void spotter_add_rotations (struct EULER a[], int n_a, struct EULER b[], int n_b, struct EULER *c[], int *n_c);
EXTERN_MSC double spotter_t2w (struct EULER a[], int n, double t);
