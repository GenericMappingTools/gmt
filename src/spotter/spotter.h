/*--------------------------------------------------------------------
 *	$Id$
 *
 *   Copyright (c) 1999-2015 by P. Wessel
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation; version 3 or any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   Contact info: www.soest.hawaii.edu/wessel
 *--------------------------------------------------------------------*/
/*
 * SPOTTER.H: Include file for programs that link with libspotter.a.
 *
 * Author:	Paul Wessel, SOEST, Univ. of Hawaii, Honolulu, HI, USA
 * Date:	19-JUL-2010
 * Version:	1.2 for GMT 5
 *
 */

/*!
 * \file spotter.h
 * \brief Include file for programs that link with libspotter.
 */

#include "gmt_dev.h" /* Requires GMT to compile and link */

#define SPOTTER_VERSION "1.2"
#define EQ_RAD 6371.0087714
#define KM_PR_DEG (EQ_RAD * M_PI / 180.0)
#define BIG_CHUNK 65536
#define T_2_PA	250.0
#define PA_2_T  (1.0 / T_2_PA)
#define SQRT_CHI2 2.44774689322	/* This is sqrt (Chi^2) for 95% and 2 degrees of freedom */

#define GPLATES_PLATES "Global_EarthByte_GPlates_Rotation_20071218.txt"
#define GPLATES_ROTATIONS "Global_EarthByte_GPlates_Rotation_20091015.rot"

/*! Structure with info on each Euler (stage) pole */
struct EULER {
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
	bool has_cov;		/* true if there is a covariance matrix for this R */
	unsigned int id[2];		/* The ID numbers for GPlates pairs */
};

/*! Structure with the nearest nodes for a single flowline */
struct FLOWLINE {
	uint64_t n;		/* Number of points in this flowline */
	uint64_t ij;		/* Node in bathymetry grid where this flowline originated */
	uint64_t *node;		/* Nodes in CVA grid covered by this flowline */
	unsigned short *PA;		/* Predicted Ages along flowline (t = PI/250, to nearest 0.004 My) */
};

/*! Structure holding all the information about a hotspot */
struct HOTSPOT {
	/* Record is lon lat abbrev id [radius toff t_on create fit plot name] */
        double lon, lat;		/* Current location of hot spot (degrees)*/
	char abbrev[4];			/* Max 3-char abbreviation of hotspot name */
        unsigned int id;		/* Hot spot id flag */
	double radius;			/* Uncertainty radius (in km) for hotspot location */
	double t_off, t_on;		/* Time interval hotspot was active */
	bool create, fit, plot;	/* true if we want to create, fit, or plot hotspot */
        char name[GMT_LEN64];	/* Full name of hotspot */
	/* Secondary (derived) quantities */
        double x, y, z;			/* Cartesian Current location of hot spot */
};

/* ANSI-C Function prototypes (see libspotter.c for details): */

EXTERN_MSC int spotter_stage (struct GMT_CTRL *GMT, double t, struct EULER p[], unsigned int ns);
EXTERN_MSC void spotter_rot_usage (struct GMTAPI_CTRL *API, char option);
EXTERN_MSC bool spotter_GPlates_pair (char *file);
EXTERN_MSC unsigned int spotter_init (struct GMT_CTRL *GMT, char *file, struct EULER **p, bool flowline, bool total_out, bool invert, double *t_max);
EXTERN_MSC unsigned int spotter_hotspot_init (struct GMT_CTRL *GMT, char *file, bool geocentric, struct HOTSPOT **p);
EXTERN_MSC unsigned int spotter_backtrack  (struct GMT_CTRL *GMT, double xp[], double yp[], double tp[], unsigned int np, struct EULER p[], unsigned int ns, double d_km, double t_zero, unsigned int do_time, double wesn[], double **c);
EXTERN_MSC unsigned int spotter_forthtrack (struct GMT_CTRL *GMT, double xp[], double yp[], double tp[], unsigned int np, struct EULER p[], unsigned int ns, double d_km, double t_zero, unsigned int do_time, double wesn[], double **c);
EXTERN_MSC void spotter_total_to_stages (struct GMT_CTRL *GMT, struct EULER p[], unsigned int n, bool total_rates, bool stage_rates);
EXTERN_MSC void spotter_stages_to_total (struct GMT_CTRL *GMT, struct EULER p[], unsigned int n, bool total_rates, bool stage_rates);
EXTERN_MSC void spotter_add_rotations (struct GMT_CTRL *GMT, struct EULER a[], int n_a, struct EULER b[], int n_b, struct EULER *c[], unsigned int *n_c);
EXTERN_MSC double spotter_t2w (struct GMT_CTRL *GMT, struct EULER a[], unsigned int n, double t);
EXTERN_MSC bool spotter_conf_ellipse (struct GMT_CTRL *GMT, double lon, double lat, double t, struct EULER *p, unsigned int np, char conf, bool forward, double out[]);
EXTERN_MSC void spotter_matrix_transpose (struct GMT_CTRL *GMT, double At[3][3], double A[3][3]);
EXTERN_MSC void spotter_matrix_add (struct GMT_CTRL *GMT, double A[3][3], double B[3][3], double C[3][3]);
EXTERN_MSC void spotter_matrix_mult (struct GMT_CTRL *GMT, double A[3][3], double B[3][3], double C[3][3]);
EXTERN_MSC void spotter_make_rot_matrix2 (struct GMT_CTRL *GMT, double E[3], double w, double R[3][3]);
EXTERN_MSC void spotter_covar_to_record (struct GMT_CTRL *GMT, struct EULER *e, double K[]);
EXTERN_MSC void spotter_cov_of_inverse (struct GMT_CTRL *GMT, struct EULER *e, double Ct[3][3]);
EXTERN_MSC void spotter_get_rotation (struct GMT_CTRL *GMT, struct EULER *p, unsigned int np, double t, double *lon, double *lat, double *w);
EXTERN_MSC void spotter_matrix_to_pole (struct GMT_CTRL *GMT, double T[3][3], double *plon, double *plat, double *w);
EXTERN_MSC void spotter_matrix_1Dto2D (struct GMT_CTRL *GMT, double *M, double X[3][3]);
EXTERN_MSC void spotter_matrix_2Dto1D (struct GMT_CTRL *GMT, double *M, double X[3][3]);
EXTERN_MSC void spotter_inv_cov (struct GMT_CTRL *GMT, double Ci[3][3], double C[3][3]);
EXTERN_MSC unsigned int spotter_confregion_radial (struct GMT_CTRL *GMT, double alpha, struct EULER *p, double **X, double **Y);
EXTERN_MSC unsigned int spotter_confregion_ortho (struct GMT_CTRL *GMT, double alpha, struct EULER *p, double **X, double **Y);
EXTERN_MSC void spotter_tangentplane (struct GMT_CTRL *GMT, double lon, double lat, double R[3][3]);
EXTERN_MSC void spotter_project_ellipsoid_new (struct GMT_CTRL *GMT, double X[3][3], double *par);
EXTERN_MSC void spotter_project_ellipsoid (struct GMT_CTRL *GMT, double axis[], double D[3][3], double *par);
EXTERN_MSC void spotter_ellipsoid_normal (struct GMT_CTRL *GMT, double X[3], double L[3], double c, double N[3]);
