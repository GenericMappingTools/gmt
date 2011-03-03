/*--------------------------------------------------------------------
 *	$Id: spotter.h,v 1.28 2011-03-03 21:02:51 guru Exp $
 *
 *   Copyright (c) 1999-2011 by P. Wessel
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 or any later version.
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
#include "gmt_proj.h"	/* For auxiliary latitude stuff */

#define SPOTTER_VERSION "1.1"
#define EQ_RAD 6371.0087714
#define KM_PR_DEG (EQ_RAD * M_PI / 180.0)
#define BIG_CHUNK 65536
#define T_2_PA	250.0
#define PA_2_T  (1.0 / T_2_PA)
#define SQRT_CHI2 2.44774689322	/* This is sqrt (Chi^2) for 95% and 2 degrees of freedom */

struct EULER {	/* Structure with info on each Euler (stage) pole */
	double lon, lat;		/* Location of Euler pole in degrees */
	double lon_r, lat_r;		/* Location of Euler pole in radians */
	double t_start, t_stop;		/* Stage beginning and end time in Ma */
	double duration;		/* Stage duration in m.y. */
	double omega;			/* Rotation in Degrees/m.y. */
	double omega_r;			/* Rotation in Radians/m.y. */
	double sin_lat, cos_lat;	/* Sine and Cosine of pole latitude */
	double C[3][3];			/* Covariance matrix for this rotation */
	double k_hat;			/* k_hat uncertainty scale */
	double g;			/* g magnitude scale */
	double df;			/* Degrees of freedom in the estimate of rotation */
	GMT_LONG has_cov;		/* TRUE if there is a covariance matrix for this R */
};

struct FLOWLINE {			/* Structure with the nearest nodes for a single flowline */
	GMT_LONG n;				/* number of points in this flowline */
	GMT_LONG ij;				/* Node in bathymetry grid where this flowline originated */
	GMT_LONG *node;			/* Nodes in CVA grid covered by this flowline */
	unsigned short *PA;		/* Predicted Ages along flowline (t = PI/250, to nearest 0.004 My) */
};

struct HOTSPOT {	/* Structure holding all the information about a hotspot */
	/* Record is lon lat abbrev id [radius toff t_on create fit plot name] */
        double lon, lat;        /* Current location of hot spot (degrees)*/
	char abbrev[4];         /* Max 3-char abbreviation of hotspot name */
        int id;                 /* Hot spot id flag */
	double radius;		/* Uncertainty radius (in km) for hotspot location */
	double t_off, t_on;	/* Time interval hotspot was active */
	GMT_LONG create, fit, plot;	/* TRUE if we want to create, fit, or plot hotspot */
        char name[32];          /* Full name of hotspot */
	/* Secondary (derived) quantities */
        double x, y, z;         /* Cartesian Current location of hot spot */
};

/* ANSI-C Function prototypes (see libspotter.c for details): */

EXTERN_MSC int spotter_init (char *file, struct EULER **p, int flowline, GMT_LONG finite_in, GMT_LONG finite_out, double *t_max, GMT_LONG verbose);
EXTERN_MSC int spotter_hotspot_init (char *file, struct HOTSPOT **p);
EXTERN_MSC int spotter_backtrack  (double xp[], double yp[], double tp[], GMT_LONG np, struct EULER p[], GMT_LONG ns, double d_km, double t_zero, GMT_LONG do_time, double wesn[], double **c);
EXTERN_MSC int spotter_forthtrack (double xp[], double yp[], double tp[], GMT_LONG np, struct EULER p[], GMT_LONG ns, double d_km, double t_zero, GMT_LONG do_time, double wesn[], double **c);
EXTERN_MSC void spotter_finite_to_stages (struct EULER p[], GMT_LONG n, GMT_LONG finite_rates, GMT_LONG stage_rates);
EXTERN_MSC void spotter_stages_to_finite (struct EULER p[], GMT_LONG n, GMT_LONG finite_rates, GMT_LONG stage_rates);
EXTERN_MSC void spotter_add_rotations (struct EULER a[], GMT_LONG n_a, struct EULER b[], GMT_LONG n_b, struct EULER *c[], GMT_LONG *n_c);
EXTERN_MSC double spotter_t2w (struct EULER a[], GMT_LONG n, double t);
EXTERN_MSC int spotter_conf_ellipse (double lon, double lat, double t, struct EULER *p, GMT_LONG np, char conf, GMT_LONG forward, double out[]);
EXTERN_MSC void spotter_matrix_vect_mult (double a[3][3], double b[3], double c[3]);
EXTERN_MSC void spotter_matrix_transpose (double At[3][3], double A[3][3]);
EXTERN_MSC void spotter_matrix_add (double A[3][3], double B[3][3], double C[3][3]);
EXTERN_MSC void spotter_matrix_mult (double A[3][3], double B[3][3], double C[3][3]);
EXTERN_MSC void spotter_make_rot_matrix (double lonp, double latp, double w, double R[3][3]);
EXTERN_MSC void spotter_covar_to_record (struct EULER *e, double K[]);
EXTERN_MSC void spotter_cov_of_inverse (struct EULER *e, double Ct[3][3]);
