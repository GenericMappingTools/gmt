/*--------------------------------------------------------------------
 *	$Id: libspotter.c,v 1.67 2011-02-23 21:49:14 guru Exp $
 *
 *   Copyright (c) 1999-2011 by P. Wessel
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
 * SPOTTER: functions for moving points along small circles on a sphere.
 *
 * Paul Wessel, University of Hawaii
 * October 24, 2001
 * Version 1.1
 *
 * The user-callable functions in this library are:
 *
 * spotter_init			: Load stage poles from file
 * spotter_backtrack		: Trace track from seamount to hotspot
 * spotter_forthtrack		: Trace track from hotspot to seamount
 * spotter_finite_to_stages	: Convert finite rotations to stage poles
 * spotter_stages_to_finite	: Convert stage poles to finite rotations
 * spotter_add_rotations	: Add to plate motion models together.
 * spotter_conf_ellipse		: Calculate confidence ellipse for rotated point
 *
 * programs must first call spotter_init() which reads a file of
 * backward stage poles.  Given the right flag it can convert these
 * to forward stage poles.
 *
 * Then to draw a hotspot track the program can:
 *	1. Draw FROM the hotspot TO a seamount: Use spotter_forthtrack
 *	2. Draw FROM a seamount BACK TO a hotspot: Use spotter_backtrack
 *
 * To draw crustal flowlines (seamounts motion over mantle) do select
 * flowline = TRUE when calling spotter_init and then:
 *	1. Draw FROM a hotspot TO a seamount: Use spotter_backtrack
 *	2. Draw FROM a seamount TO a hotspot (and beyond): Use spotter_forthtrack
 *
 * All coordinates herein are assumed to be GEOCENTRIC.  The main programs are
 * respondible for coverting to/from geodetic data coordinates.  Rotation pole
 * latitudes are usually implied to be geocentric.
 */

#include "spotter.h"

/* Internal functions */

void matrix_to_pole (double T[3][3], double *plon, double *plat, double *w);
void make_rot_matrix_sub (double E[3], double w, double R[3][3]);
void reverse_rotation_order (struct EULER *p, GMT_LONG n);
void xyw_to_struct_euler (struct EULER *p, double lon[], double lat[], double w[], GMT_LONG n, GMT_LONG stages, GMT_LONG convert);
void set_I_matrix (double R[3][3]);
GMT_LONG must_do_track (GMT_LONG sideA[], GMT_LONG sideB[]);
void set_inout_sides (double x, double y, double wesn[], GMT_LONG sideXY[2]);
void record_to_covar (struct EULER *e, double K[8]);
void covar_of_transpose (double A[3][3], double C[3][3], double Ct[3][3]);
void spotter_set_M (double lon, double lat, double M[3][3]);

void spotter_finite_to_fwstages (struct EULER p[], GMT_LONG n, GMT_LONG finite_rates, GMT_LONG stage_rates);

int spotter_init (char *file, struct EULER **p, int flowline, GMT_LONG finite_in, GMT_LONG finite_out, double *t_max, GMT_LONG verbose)
{
	/* file;	Name of file with backward stage poles */
	/* p;		Pointer to stage pole array */
	/* flowline;	TRUE if flowlines rather than hotspot-tracks are needed */
	/* finite_in;	TRUE for finite (total construction poles) files [alternative is stage poles] */
	/* finite_out;	TRUE if we want to return finite (total construction poles) [alternative is stage poles]*/
	/* t_max;	Extend earliest stage pole back to this age */
	FILE *fp;
	struct EULER *e;
	char  buffer[BUFSIZ];
	GMT_LONG n, nf, i = 0, k, n_alloc = GMT_SMALL_CHUNK;
	double K[9];
	int spotter_comp_stage (const void *p_1, const void *p_2);
	int spotter_comp_finite (const void *p_1, const void *p_2);

	e = (struct EULER *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (struct EULER), "libspotter");

	if ((fp = GMT_fopen (file, "r")) == NULL) {
		fprintf (stderr, "libspotter: ERROR: Cannot open stage pole file: %s\n", file);
		GMT_exit (EXIT_FAILURE);
	}

	if (flowline) finite_out = TRUE;	/* Override so we get finite poles for conversion to forward stage poles at the end */

	while (GMT_fgets (buffer, 512, fp) != NULL) { /* Expects lon lat t0 t1 ccw-angle */
		GMT_chop (buffer);					/* Rid the world of CR/LF */
		if (buffer[0] == '#' || buffer[0] == '\0') continue;

		if (finite_in) {	/* The minimalist record formats is: lon lat t0 [t1] omega [covar] */
			nf = sscanf (buffer, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&e[i].lon, &e[i].lat, &e[i].t_start, &e[i].t_stop, &e[i].omega, &K[0], &K[1], &K[2], &K[3], &K[4], &K[5], &K[6], &K[7], &K[8]);
			if (! (nf == 4 || nf == 5 || nf == 13 || nf == 14)) {
				fprintf (stderr, "libspotter: ERROR: Rotation file format must be lon lat t0 [t1] omega [k_hat a b c d e f g df]\n");
				GMT_exit (EXIT_FAILURE);
			}
			if (nf == 4 || nf == 13) {	/* Got lon lat t0 omega [covars], must shift the K's by one */
				for (k = 8; k > 0; k--) K[k] = K[k-1];
				K[0] = e[i].omega;
				e[i].omega = e[i].t_stop;
				e[i].t_stop = 0.0;
			}
			if (nf > 5) { /* [K = covars] is stored as [k_hat a b c d e f g df] */
				if (K[8] == 0.0) K[8] = 10000.0;	/* No d.f. given */
				record_to_covar (&e[i], K);
			}
		}
		else {	/* Stage rotations */
			nf = sscanf (buffer, "%lf %lf %lf %lf %lf", &e[i].lon, &e[i].lat, &e[i].t_start, &e[i].t_stop, &e[i].omega);
		}


		if (e[i].t_stop >= e[i].t_start) {
			fprintf (stderr, "libspotter: ERROR: (%s) Stage rotation %ld has start time younger than stop time\n", file, i);
			GMT_exit (EXIT_FAILURE);
		}
		e[i].duration = e[i].t_start - e[i].t_stop;
		e[i].omega /= e[i].duration;	/* Convert to opening rate */

		e[i].omega_r = e[i].omega * D2R;
		sincosd (e[i].lat, &e[i].sin_lat, &e[i].cos_lat);
		e[i].lon_r = e[i].lon * D2R;
		e[i].lat_r = e[i].lat * D2R;
		i++;
		if (i == n_alloc) {
			n_alloc <<= 1;
			e = (struct EULER *) GMT_memory ((void *)e, (size_t)n_alloc, sizeof (struct EULER), "libspotter");
		}
	}
	GMT_fclose (fp);

	/* Sort the rotations to make sure they are in the expected order */
	
	n = i;
	if (finite_in)
		qsort ((void *)e, (size_t)n, sizeof (struct EULER), spotter_comp_finite);
	else
		qsort ((void *)e, (size_t)n, sizeof (struct EULER), spotter_comp_stage);
	
	if (finite_in && !finite_out) spotter_finite_to_stages (e, n, TRUE, TRUE);	/* Convert total reconstruction poles to forward stage poles */
	if (!finite_in && finite_out) spotter_stages_to_finite (e, n, TRUE, TRUE);	/* Convert forward stage poles to total reconstruction poles */

	e = (struct EULER *) GMT_memory ((void *)e, (size_t)n, sizeof (struct EULER), "libspotter");

	if (flowline) {	/* Get the forward stage poles from the total reconstruction poles */
		spotter_finite_to_fwstages (e, n, TRUE, TRUE);
	}

	/* Extend oldest stage pole back to t_max Ma */

	if ((*t_max) > 0.0 && e[0].t_start < (*t_max)) {
		if (verbose) fprintf (stderr, "libspotter: (%s) Extending oldest stage pole back to %g Ma\n", file, (*t_max));

		e[0].t_start = (*t_max);
		e[0].duration = e[0].t_start - e[0].t_stop;
	}
	else
		(*t_max) = MAX (e[0].t_start, e[n-1].t_start);
	*p = e;

	return (n);
}

/* Sort functions used to order the rotations */

int spotter_comp_stage (const void *p_1, const void *p_2)
{
	/* Returns -1 if rotation pointed to by p_1 is older that point_2,
	   +1 if the reverse it true, and 0 if they are equal
	*/
	struct EULER *point_1, *point_2;

	point_1 = (struct EULER *)p_1;
	point_2 = (struct EULER *)p_2;

	if (point_1->t_start > point_2->t_start) return (-1);
	if (point_1->t_start < point_2->t_start) return (1);
	return (0);
}

int spotter_comp_finite (const void *p_1, const void *p_2)
{
	/* Returns -1 if rotation pointed to by p_1 is older that point_2,
	   +1 if the reverse it true, and 0 if they are equal
	*/
	struct EULER *point_1, *point_2;

	point_1 = (struct EULER *)p_1;
	point_2 = (struct EULER *)p_2;

	if (point_1->t_start < point_2->t_start) return (-1);
	if (point_1->t_start > point_2->t_start) return (1);
	return (0);
}
/* hotspot_init: Reads a file with hotspot information and returns pointer to
 * array of structures */

int spotter_hotspot_init (char *file, struct HOTSPOT **p)
{
	FILE *fp;
	struct HOTSPOT *e;
	char buffer[BUFSIZ], create, fit, plot;
	int i = 0, n;
	size_t n_alloc = GMT_CHUNK;
	double P[3];

	if ((fp = GMT_fopen (file, "r")) == NULL) {
		fprintf (stderr, "%s: Cannot open file %s - aborts\n", "libspotter", file);
		exit (EXIT_FAILURE);
	}

	e = (struct HOTSPOT *) GMT_memory (VNULL, n_alloc, sizeof (struct HOTSPOT), "libspotter");

	while (GMT_fgets (buffer, 512, fp) != NULL) {
		GMT_chop (buffer);					/* Rid the world of CR/LF */
		if (buffer[0] == '#' || buffer[0] == '\0') continue;
		n = sscanf (buffer, "%lf %lf %s %d %lf %lf %lf %c %c %c %s", &e[i].lon, &e[i].lat, e[i].abbrev, &e[i].id, &e[i].radius, &e[i].t_off, &e[i].t_on, &create, &fit, &plot, e[i].name);
		if (n == 3) e[i].id = i;	/* Minimal lon, lat, abbrev */
		if (n >= 10) {
			e[i].create = (create == 'Y');
			e[i].fit = (fit == 'Y');
			e[i].plot = (plot == 'Y');
		}
		GMT_geo_to_cart (e[i].lat, e[i].lon, P, TRUE);
		e[i].x = P[0];
		e[i].y = P[1];
		e[i].z = P[2];
		i++;
		if ((size_t)i == n_alloc) {
			n_alloc <<= 1;
			e = (struct HOTSPOT *) GMT_memory ((void *)e, n_alloc, sizeof (struct HOTSPOT), "libspotter");
		}
	}
	GMT_fclose (fp);
	e = (struct HOTSPOT *) GMT_memory ((void *)e, (size_t)i, sizeof (struct HOTSPOT), "libspotter");
	*p = e;

	return (i);
}

/* spotter_backtrack: Given a seamount location and age, trace the
 *	hotspot-track between this seamount and a seamount of 
 *	age t_zero.  For t_zero = 0 this means the hotspot
 */

int spotter_backtrack (double xp[], double yp[], double tp[], GMT_LONG np, struct EULER p[], GMT_LONG ns, double d_km, double t_zero, GMT_LONG do_time, double wesn[], double **c)
/* xp, yp;	Points, in RADIANS */
/* tp;		Age of feature in m.y. */
/* np;		# of points */
/* p;		Stage poles */
/* ns;		# of stage poles */
/* d_km;	Create track point every d_km km.  If == -1.0, return bend points only */
/* t_zero;	Backtrack up to this age */
/* do_time;	TRUE if we want to interpolate and return time along track, 2 if we just want stage # */
/* wesn:	if do_time >= 10, only to track within the given box */
/* **c;		Pointer to return track vector */
{
	GMT_LONG i, j = 0, k, kk = 0, start_k = 0, nd = 1, nn, n_alloc = 2 * GMT_CHUNK, sideA[2], sideB[2];
	GMT_LONG path, bend, go = FALSE, box_check;
	double t, tt = 0.0, dt, d_lon, tlon, dd = 0.0, i_km = 0.0, xnew, xx, yy, next_x, next_y;
	double s_lat, c_lat, s_lon, c_lon, cc, ss, cs, i_nd, *track = VNULL;

	bend = (d_km <= (GMT_SMALL - 1.0));
	path = (bend || d_km > GMT_SMALL);
	if (do_time >= 10) {	/* Restrict track sampling to given wesn box */
		do_time -= 10;
		box_check = TRUE;
	}
	else {
		box_check = FALSE;
		go = TRUE;
	}

	if (path) {
		track = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "libspotter");
		i_km = EQ_RAD / d_km;
	}

	if (p[ns-1].t_stop > t_zero) t_zero = p[ns-1].t_stop;	/* In case we don't go all the way to zero */

	for (i = 0; i < np; i++) {

		if (path) {
			start_k = kk++;
			if (kk == n_alloc) {
				n_alloc <<= 1;
				track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
			}
		}
		nn = 0;

		t = tp[i];

		if (box_check) set_inout_sides (xp[i], yp[i], wesn, sideB);
		while (t > t_zero) {	/* As long as we're not back at zero age */
			if (box_check) sideA[0] = sideB[0], sideA[1] = sideB[1];

			j = 0;
			while (j < ns && t <= p[j].t_stop) j++;	/* Find first applicable stage pole */
			if (j == ns) {
				fprintf (stderr, "libspotter: (spotter_backtrack) Ran out of stage poles for t = %g\n", t);
				GMT_exit (EXIT_FAILURE);
			}
			dt = MIN (p[j].duration, t - MAX(p[j].t_stop, t_zero));
			d_lon = p[j].omega_r * dt;

			xnew = xp[i] - p[j].lon_r;
			sincos (yp[i], &s_lat, &c_lat);
			sincos (xnew, &s_lon, &c_lon);
			cc = c_lat * c_lon;
			tlon = d_atan2 (c_lat * s_lon, p[j].sin_lat * cc - p[j].cos_lat * s_lat);
			s_lat = p[j].sin_lat * s_lat + p[j].cos_lat * cc;
			c_lat = sqrt (1.0 - s_lat * s_lat);
			ss = p[j].sin_lat * s_lat;
			cs = p[j].cos_lat * s_lat;

			/* Get the next bend point first */

			xnew = tlon + d_lon;
			sincos (xnew, &s_lon, &c_lon);
			cc = c_lat * c_lon;
			next_y = d_asin (ss - p[j].cos_lat * cc);
			next_x = p[j].lon_r + d_atan2 (c_lat * s_lon, p[j].sin_lat * cc + cs);

			if (next_x < 0.0) next_x += TWO_PI;
			if (next_x >= TWO_PI) next_x -= TWO_PI;

			if (box_check) {	/* See if this segment _might_ involve the box in any way; if so do the track sampling */
				set_inout_sides (next_x, next_y, wesn, sideB);
				go = must_do_track (sideA, sideB);
			}
			if (path) {
				if (!bend) {
					nd = (GMT_LONG) ceil ((fabs (d_lon) * c_lat) * i_km);
					i_nd = 1.0 / nd;
					dd = d_lon * i_nd;
					tt = dt * i_nd;
				}
				track[kk++] = xp[i];
				if (kk == n_alloc) {
					n_alloc <<= 1;
					track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
				}
				track[kk++] = yp[i];
				if (kk == n_alloc) {
					n_alloc <<= 1;
					track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
				}
				if (do_time) {
					track[kk++] = (do_time == 2) ? (double)(ns - j) : t;
					if (kk == n_alloc) {
						n_alloc <<= 1;
						track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
					}
				}
				if (!go) nd = 1;
				for (k = 1; go && k < nd; k++) {

					xnew = tlon + k * dd;
					sincos (xnew, &s_lon, &c_lon);
					cc = c_lat * c_lon;
					yy = d_asin (ss - p[j].cos_lat * cc);
					xx = p[j].lon_r + d_atan2 (c_lat * s_lon, p[j].sin_lat * cc + cs);

					if (xx < 0.0) xx += TWO_PI;
					if (xx >= TWO_PI) xx -= TWO_PI;
					track[kk++] = xx;
					if (kk == n_alloc) {
						n_alloc <<= 1;
						track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
					}
					track[kk++] = yy;
					if (kk == n_alloc) {
						n_alloc <<= 1;
						track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
					}
					if (do_time) {
						track[kk++] = (do_time == 2) ? (double)(ns - j) : t - k * tt;
						if (kk == n_alloc) {
							n_alloc <<= 1;
							track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
						}
					}
				}
				nn += nd;
			}
			xp[i] = next_x;	yp[i] = next_y;
			t -= dt;
		}
		if (path) {
			track[kk++] = xp[i];
			if (kk == n_alloc) {
				n_alloc <<= 1;
				track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
			}
			track[kk++] = yp[i];
			if (kk == n_alloc) {
				n_alloc <<= 1;
				track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
			}
			if (do_time) {
				track[kk++] = (do_time == 2) ? (double)(ns - j) : t;
				if (kk == n_alloc) {
					n_alloc <<= 1;
					track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
				}
			}
			track[start_k] = nn+1;
		}
	}
	if (path) {
		track = (double *) GMT_memory ((void *)track, (size_t)kk, sizeof (double), "libspotter");
		*c = track;
		return (kk);
	}

	return (np);
}

void set_inout_sides (double x, double y, double wesn[], GMT_LONG sideXY[2]) {
	/* Given the rectangular region in wesn, return -1, 0, +1 for
	 * x and y if the point is left/below (-1) in (0), or right/above (+1).
	 * 
	 */
	 
	if (y < wesn[2])
		sideXY[1] = -1;
	else if (y > wesn[3])
		sideXY[1] = +1;
	else
		sideXY[1] = 0;
	while ((x + TWO_PI) < wesn[1]) x += TWO_PI;
	while ((x - TWO_PI) > wesn[0]) x -= TWO_PI;
	if (x < wesn[0])
		sideXY[0] = -1;
	else if (x > wesn[1])
		sideXY[0] = +1;
	else
		sideXY[0] = 0;
}

GMT_LONG must_do_track (GMT_LONG sideA[], GMT_LONG sideB[]) {
	GMT_LONG dx, dy;
	/* First check if any of the two points are inside the box */
	if (sideA[0] == 0 && sideA[1] == 0) return (TRUE);
	if (sideB[0] == 0 && sideB[1] == 0) return (TRUE);
	/* Now check if the two points may cut a corner */
	dx = GMT_abs (sideA[0] - sideB[0]);
	dy = GMT_abs (sideA[1] - sideB[1]);
	if (dx && dy) return (TRUE);
	if (dx == 2 || dy == 2) return (TRUE);	/* Could cut across the box */
	return (FALSE);
}

/* spotter_forthtrack: Given a hotspot location and final age, trace the
 *	hotspot-track between the seamount created at t_zero and a
 *	seamount of age tp.  For t_zero = 0 this means from the hotspot.
 */

int spotter_forthtrack (double xp[], double yp[], double tp[], GMT_LONG np, struct EULER p[], GMT_LONG ns, double d_km, double t_zero, GMT_LONG do_time, double wesn[], double **c)
/* xp, yp;	Points, in RADIANS */
/* tp;		Age of feature in m.y. */
/* np;		# of points */
/* p;		Stage poles */
/* ns;		# of stage poles */
/* d_km;	Create track point every d_km km.  If == -1.0, return bend points only */
/* t_zero;	Foretrack from this age forward */
/* do_time;	TRUE if we want to interpolate and return time along track */
/* wesn:	if do_time >= 10, only to track within the given box */
/* c;		Pointer to return track vector */
{
	GMT_LONG i, j = 0, k, kk = 0, start_k = 0, nd = 1, nn, n_alloc = BIG_CHUNK, sideA[2], sideB[2];
	GMT_LONG path, bend, go = FALSE, box_check;
	double t, tt = 0.0, dt, d_lon, tlon, dd = 0.0, i_km = 0.0, xnew, xx, yy, *track = VNULL;
	double s_lat, c_lat, s_lon, c_lon, cc, ss, cs, i_nd, next_x, next_y;

	bend = (d_km <= (GMT_SMALL - 1.0));
	path = (bend || d_km > GMT_SMALL);
	if (do_time >= 10) {	/* Restrict track sampling to given wesn box */
		do_time -= 10;
		box_check = TRUE;
	}
	else {
		box_check = FALSE;
		go = TRUE;
	}

	if (path) {
		track = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "libspotter");
		i_km = EQ_RAD / d_km;
	}

	if (p[ns-1].t_stop > t_zero) t_zero = p[ns-1].t_stop;	/* In case we don't go all the way to zero */

	for (i = 0; i < np; i++) {

		if (path) {
			start_k = kk++;
			if (kk == n_alloc) {
				n_alloc <<= 1;
				track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
			}
		}
		nn = 0;

		t = t_zero;

		if (box_check) set_inout_sides (xp[i], yp[i], wesn, sideB);
		while (t < tp[i]) {	/* As long as we're not back at zero age */
			if (box_check) sideA[0] = sideB[0], sideA[1] = sideB[1];
			j = ns - 1;
			while (j && (t + GMT_CONV_LIMIT) > p[j].t_start) j--;
			/* while (j < ns && (t + GMT_CONV_LIMIT) < p[j].t_stop) j++; */	/* Find first applicable stage pole */
			if (j == ns) {
				fprintf (stderr, "libspotter: (spotter_forthtrack) Ran out of stage poles for t = %g\n", t);
				GMT_exit (EXIT_FAILURE);
			}
			dt = MIN (tp[i], p[j].t_start) - t;	/* Time interval to rotate */
			d_lon = p[j].omega_r * dt;		/* Rotation angle (radians) */

			xnew = xp[i] - p[j].lon_r;
			sincos (yp[i], &s_lat, &c_lat);
			sincos (xnew, &s_lon, &c_lon);
			cc = c_lat * c_lon;
			tlon = d_atan2 (c_lat * s_lon, p[j].sin_lat * cc - p[j].cos_lat * s_lat);
			s_lat = p[j].sin_lat * s_lat + p[j].cos_lat * cc;
			c_lat = sqrt (1.0 - s_lat * s_lat);
			ss = p[j].sin_lat * s_lat;
			cs = p[j].cos_lat * s_lat;

			/* Get the next bend point first */

			xnew = tlon - d_lon;
			sincos (xnew, &s_lon, &c_lon);
			cc = c_lat * c_lon;
			next_y = d_asin (ss - p[j].cos_lat * cc);
			next_x = p[j].lon_r + d_atan2 (c_lat * s_lon, p[j].sin_lat * cc + cs);

			if (next_x < 0.0) next_x += TWO_PI;
			if (next_x >= TWO_PI) next_x -= TWO_PI;

			if (box_check) {	/* See if this segment _might_ involve the box in any way; if so do the track sampling */
				set_inout_sides (next_x, next_y, wesn, sideB);
				go = must_do_track (sideA, sideB);
			}
			if (path) {
				if (!bend) {
					nd = (GMT_LONG) ceil ((fabs (d_lon) * c_lat) * i_km);
					i_nd = 1.0 / nd;
					dd = d_lon * i_nd;
					tt = dt * i_nd;
				}
				track[kk++] = xp[i];
				if (kk == n_alloc) {
					n_alloc <<= 1;
					track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
				}
				track[kk++] = yp[i];
				if (kk == n_alloc) {
					n_alloc <<= 1;
					track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
				}
				if (do_time) {
					track[kk++] = (do_time == 2) ? (double)(ns - j) : t;
					if (kk == n_alloc) {
						n_alloc <<= 1;
						track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
					}
				}
				if (!go) nd = 1;
				for (k = 1; go && k < nd; k++) {
					xnew = tlon - k * dd;
					sincos (xnew, &s_lon, &c_lon);
					cc = c_lat * c_lon;
					yy = d_asin (ss - p[j].cos_lat * cc);
					xx = p[j].lon_r + d_atan2 (c_lat * s_lon, p[j].sin_lat * cc + cs);

					if (xx < 0.0) xx += TWO_PI;
					if (xx >= TWO_PI) xx -= TWO_PI;
					track[kk++] = xx;
					if (kk == n_alloc) {
						n_alloc <<= 1;
						track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
					}
					track[kk++] = yy;
					if (kk == n_alloc) {
						n_alloc <<= 1;
						track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
					}
					if (do_time) {
						track[kk++] = (do_time == 2) ? (double)(ns - j) : t + k * tt;
						if (kk == n_alloc) {
							n_alloc <<= 1;
							track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
						}
					}
				}
				nn += nd;
			}

			xp[i] = next_x;	yp[i] = next_y;
			t += dt;
		}
		if (path) {
			track[kk++] = xp[i];
			if (kk == n_alloc) {
				n_alloc <<= 1;
				track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
			}
			track[kk++] = yp[i];
			if (kk == n_alloc) {
				n_alloc <<= 1;
				track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
			}
			if (do_time) {
				track[kk++] = (do_time == 2) ? (double)(ns - j) : t;
				if (kk == n_alloc) {
					n_alloc <<= 1;
					track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
				}
			}
			track[start_k] = nn+1;
		}
	}
	if (path) {
		track = (double *) GMT_memory ((void *)track, (size_t)kk, sizeof (double), "libspotter");
		*c = track;
		return (kk);
	}

	return (np);
}

/* Converts a set of total reconstruction poles to forward stage poles for flowlines
 *
 * Based partly on Cox and Hart, 1986
 */

void spotter_finite_to_fwstages (struct EULER p[], GMT_LONG n, GMT_LONG finite_rates, GMT_LONG stage_rates)
{
	/* Convert finite rotations to forward stage rotations for flowlines */
	/* p[]		: Array of structure elements with rotation parameters
	 * n		: Number of rotations
	 * finite_rates	: TRUE if finite rotations given in degree/my [else we have opening angle]
	 * stage_rates	: TRUE if stage rotations should be returned in degree/my [else we return opening angle]
	 */
	 
	GMT_LONG i;
	double *elon, *elat, *ew, t_old;
	double R_young[3][3], R_old[3][3], R_stage[3][3];

	/* Expects total reconstruction models to have youngest poles first */

	elon = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "libspotter");
	elat = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "libspotter");
	ew   = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "libspotter");

	set_I_matrix (R_young);		/* The first time, R_young is simply I */

	/* First forward stage pole is the youngest total reconstruction pole */

	t_old = 0.0;
	for (i = 0; i < n; i++) {
		if (finite_rates) p[i].omega *= p[i].duration;			/* Convert opening rate to opening angle */
		spotter_make_rot_matrix (p[i].lon, p[i].lat, -p[i].omega, R_old);	/* Make rotation matrix from rotation parameters, take transpose by passing -omega */
		spotter_matrix_mult (R_young, R_old, R_stage);				/* This is R_stage = R_young * R_old^t */
		matrix_to_pole (R_stage, &elon[i], &elat[i], &ew[i]);		/* Get rotation parameters from matrix */
		if (elon[i] > 180.0) elon[i] -= 360.0;				/* Adjust lon */
		spotter_matrix_transpose (R_young, R_old);				/* Set R_young = (R_old^t)^t = R_old */
		p[i].t_stop = t_old;
		t_old = p[i].t_start;
	}

	/* Repopulate the EULER structure given the rotation parameters */

	xyw_to_struct_euler (p, elon, elat, ew, n, TRUE, stage_rates);

	GMT_free ((void *)elon);
	GMT_free ((void *)elat);
	GMT_free ((void *)ew);

	/* Flip order since stages go from oldest to youngest */

	reverse_rotation_order (p, n);	/* Flip order since stages go from oldest to youngest */
}

void spotter_finite_to_stages (struct EULER p[], GMT_LONG n, GMT_LONG finite_rates, GMT_LONG stage_rates)
{
	/* Convert finite rotations to backwards stage rotations for backtracking */
	/* p[]		: Array of structure elements with rotation parameters
	 * n		: Number of rotations
	 * finite_rates	: TRUE if finite rotations given in degree/my [else we have opening angle]
	 * stage_rates	: TRUE if stage rotations should be returned in degree/my [else we return opening angle]
	 */
	 
	GMT_LONG i;
	double *elon, *elat, *ew, t_old;
	double R_young[3][3], R_old[3][3], R_stage[3][3];

	/* Expects total reconstruction models to have youngest poles first */

	elon = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "libspotter");
	elat = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "libspotter");
	ew   = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "libspotter");

	set_I_matrix (R_young);		/* The first time, R_young is simply I */

	t_old = 0.0;
	for (i = 0; i < n; i++) {
		if (finite_rates) p[i].omega *= p[i].duration;			/* Convert opening rate to opening angle */
		spotter_make_rot_matrix (p[i].lon, p[i].lat, p[i].omega, R_old);	/* Get rotation matrix from pole and angle */
		spotter_matrix_mult (R_young, R_old, R_stage);				/* This is R_stage = R_young^t * R_old */
		matrix_to_pole (R_stage, &elon[i], &elat[i], &ew[i]);		/* Get rotation parameters from matrix */
		if (elon[i] > 180.0) elon[i] -= 360.0;				/* Adjust lon */
		spotter_matrix_transpose (R_young, R_old);				/* Sets R_young = transpose (R_old) for next round */
		p[i].t_stop = t_old;
		t_old = p[i].t_start;
	}

	/* Repopulate the EULER structure given the rotation parameters */

	xyw_to_struct_euler (p, elon, elat, ew, n, TRUE, stage_rates);

	GMT_free ((void *)elon);
	GMT_free ((void *)elat);
	GMT_free ((void *)ew);

	reverse_rotation_order (p, n);	/* Flip order since stages go from oldest to youngest */
}

void spotter_stages_to_finite (struct EULER p[], GMT_LONG n, GMT_LONG finite_rates, GMT_LONG stage_rates)
{
	/* Convert stage rotations to finite rotations */
	/* p[]		: Array of structure elements with rotation parameters
	 * n		: Number of rotations
	 * finite_rates	: TRUE if finite rotations should be returned in degree/my [else we return opening angle]
	 * stage_rates	: TRUE if stage rotations given in degree/my [else we have opening angle]
	 */

	GMT_LONG i;
	double *elon, *elat, *ew;
	double R_young[3][3], R_old[3][3], R_stage[3][3];

	/* Expects stage pole models to have oldest poles first, so we must flip order */

	reverse_rotation_order (p, n);	/* Expects stage pole models to have oldest poles first, so we must flip order */

	elon = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "libspotter");
	elat = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "libspotter");
	ew   = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "libspotter");

	set_I_matrix (R_old);		/* The first time, R_old is simply I */

	for (i = 0; i < n; i++) {
		if (stage_rates) p[i].omega *= p[i].duration;				/* Convert opening rate to opening angle */
		spotter_make_rot_matrix (p[i].lon, p[i].lat, p[i].omega, R_stage);		/* Make matrix from rotation parameters */
		spotter_matrix_mult (R_old, R_stage, R_young);					/* Set R_young = R_old * R_stage */
		memcpy ((void *)R_old, (void *)R_young, (size_t)(9 * sizeof (double)));	/* Set R_old = R_young for next time around */
		matrix_to_pole (R_young, &elon[i], &elat[i], &ew[i]);			/* Get rotation parameters from matrix */
		if (elon[i] > 180.0) elon[i] -= 360.0;					/* Adjust lon */
	}

	/* Repopulate the EULER structure given the rotation parameters */

	xyw_to_struct_euler (p, elon, elat, ew, n, FALSE, finite_rates);

	GMT_free ((void *)elon);
	GMT_free ((void *)elat);
	GMT_free ((void *)ew);
}

void spotter_add_rotations (struct EULER a[], GMT_LONG n_a, struct EULER b[], GMT_LONG n_b, struct EULER *c[], GMT_LONG *n_c)
{
	/* Takes two finite rotation models and adds them together.
	 * We do this by first converting both to stage poles.  We then
	 * determine all the time knots needed, and then resample both
	 * stage rotation models onto the same list of knots.  This is
	 * easy to do with stage poles since we end up using partial
	 * stage rotations.  TO do this with finite poles would involve
	 * computing intermediate stages anyway.  When we have the resampled
	 * stage rotations we convert back to finite rotations and then
	 * simply add each pair of rotations using matrix multiplication.
	 * The final finite rotation model is returned in c. */

	struct EULER *a2, *b2, *c2;
	double *t, t_min, t_max, Ra[3][3], Rb[3][3], Rab[3][3], lon, lat, w, sign_a, sign_b;
	double tmp[3][3], RaT[3][3], Ca[3][3], Cb[3][3];
	GMT_LONG i, j, k, n_k = 0;
	GMT_LONG a_ok = TRUE, b_ok = TRUE;

	sign_a = (n_a > 0) ? +1.0 : -1.0;
	sign_b = (n_b > 0) ? +1.0 : -1.0;
	n_a = GMT_abs (n_a);
	n_b = GMT_abs (n_b);
	/* Allocate more than we need, must likely */

	t = (double *) GMT_memory (VNULL, (size_t)(n_a + n_b), sizeof (double), "libspotter");

	/* First convert the two models to stage poles */

	spotter_finite_to_stages (a, n_a, TRUE, TRUE);		/* Return stage poles */
	spotter_finite_to_stages (b, n_b, TRUE, TRUE);		/* Return stage poles */

	/* Find all the time knots used by the two models */

	t_max = MIN (a[0].t_start, b[0].t_start);
	t_min = MAX (a[n_a-1].t_stop, b[n_b-1].t_stop);
	t[n_k++] = t_max;
	i = j = 0;
	while (i < n_a && a[i].t_stop >= t[0]) i++;
	if (i == (n_a - 1)) a_ok = FALSE;
	while (j < n_b && b[j].t_stop >= t[0]) j++;
	if (j == (n_b - 1)) b_ok = FALSE;
	while (a_ok || b_ok) {
		if (a_ok && !b_ok) {		/* Only a left */
			t[n_k] = a[i++].t_stop;
			if (i == (n_a - 1)) a_ok = FALSE;
		}
		else if (b_ok && !a_ok) {	/* Only b left */
			t[n_k] = b[j++].t_stop;
			if (j == (n_b - 1)) b_ok = FALSE;
		}
		else if (a_ok && a[i].t_stop > b[j].t_stop) {
			t[n_k] = a[i++].t_stop;
			if (i == (n_a - 1)) a_ok = FALSE;
		}
		else if (b_ok && b[j].t_stop > a[i].t_stop) {
			t[n_k] = b[j++].t_stop;
			if (j == (n_b - 1)) b_ok = FALSE;
		}
		else {	/* Same time for both */
			t[n_k] = b[j++].t_stop;
			i++;
			if (i == (n_a - 1)) a_ok = FALSE;
			if (j == (n_b - 1)) b_ok = FALSE;
		}
		n_k++;
	}
	t[n_k++] = t_min;
	n_k--;	/* Number of structure elements is one less than number of knots */

	b2 = (struct EULER *) GMT_memory (VNULL, (size_t)n_k, sizeof (struct EULER), "libspotter");
	a2 = (struct EULER *) GMT_memory (VNULL, (size_t)n_k, sizeof (struct EULER), "libspotter");
	c2 = (struct EULER *) GMT_memory (VNULL, (size_t)n_k, sizeof (struct EULER), "libspotter");

	for (k = i = j = 0; k < n_k; k++) {	/* Resample the two stage pole models onto the same knots */
		/* First resample p onto p2 */
		while (a[i].t_stop >= t[k]) i++;				/* Wind up */
		a2[k] = a[i];							/* First copy everything */
		if (a2[k].t_start > t[k]) a2[k].t_start = t[k];			/* Adjust start time */
		if (a2[k].t_stop < t[k+1]) a2[k].t_stop = t[k+1];		/* Adjust stop time */
		a2[k].duration = a2[k].t_start - a2[k].t_stop;			/* Set the duration */

		/* Then resample a onto a2 */
		while (b[j].t_stop >= t[k]) j++;				/* Wind up */
		b2[k] = b[j];							/* First copy everything */
		if (b2[k].t_start > t[k]) b2[k].t_start = t[k];			/* Adjust start time */
		if (b2[k].t_stop < t[k+1]) b2[k].t_stop = t[k+1];		/* Adjust stop time */
		b2[k].duration = b2[k].t_start - b2[k].t_stop;			/* Set the duration */
	}

	GMT_free ((void *)t);

	/* Now switch to finite rotations again to do the additions */

	spotter_stages_to_finite (a2, n_k, FALSE, TRUE);	/* Return opening angles, not rates this time */
	spotter_stages_to_finite (b2, n_k, FALSE, TRUE);

	for (i = 0; i < n_k; i++) {	/* Add each pair of rotations */
		spotter_make_rot_matrix (a2[i].lon, a2[i].lat, sign_a * a2[i].omega, Ra);
		spotter_make_rot_matrix (b2[i].lon, b2[i].lat, sign_b * b2[i].omega, Rb);
		spotter_matrix_mult (Rb, Ra, Rab);	/* Rot a + Rot b = RB * Ra ! */
		matrix_to_pole (Rab, &lon, &lat, &w);
		c2[i].lon = lon;
		c2[i].lat = lat;
		c2[i].t_start = a2[i].t_start;
		c2[i].t_stop  = 0.0;
		c2[i].duration = c2[i].t_start;
		c2[i].omega = w / c2[i].duration;	/* Return rates again */
		if (a2[i].has_cov && b2[i].has_cov) {	/* May compute combined covariance matrix, assuming khats = 1 */
			double fa, fb;
			c2[i].df = a2[i].df + b2[i].df;
			if (a2[i].k_hat != 1.0 || b2[i].k_hat != 1.0) {	/* Kappas are not one, use pooled estimate */
				c2[i].k_hat = (c2[i].df / (a2[i].df / a2[i].k_hat + b2[i].df / b2[i].k_hat));
				fa = a2[i].k_hat / c2[i].k_hat;
				fb = b2[i].k_hat / c2[i].k_hat;
			}
			else
				c2[i].k_hat = fa = fb = 1.0;

			spotter_matrix_transpose (RaT, Ra);
			if (sign_a < 0.0)
				spotter_cov_of_inverse (&a2[i], Ca);
			else
				memcpy ((void *)Ca, (void *)a2[i].C, 9*sizeof (double));
			if (sign_b < 0.0)
				spotter_cov_of_inverse (&b2[i], Cb);
			else
				memcpy ((void *)Cb, (void *)b2[i].C, 9*sizeof (double));
			spotter_matrix_mult (Cb, Ra, tmp);
			spotter_matrix_mult (RaT, tmp, c2[i].C);
			for (k = 0; k < 3; k++) for (j = 0; j < 3; j++) c2[i].C[k][j] *= fb;
			for (k = 0; k < 3; k++) for (j = 0; j < 3; j++) tmp[k][j] = fa * Ca[k][j];
			spotter_matrix_add (c2[i].C, tmp, c2[i].C);
			c2[i].has_cov = TRUE;
			c2[i].g = MIN(a2[i].g, b2[i].g);
		}
	}
	GMT_free ((void *)a2);
	GMT_free ((void *)b2);

	*n_c = n_k;
	*c = c2;
}

double spotter_t2w (struct EULER a[], GMT_LONG n, double t)
{
	/* Take time, return cumulative omega */

	GMT_LONG i;
	double w = 0.0;

	i = n - 1;
	while (i >= 0 && t > a[i].t_start) {
		w += fabs (a[i].omega * a[i].duration);
		i--;
	}
	if (i >= 0 && t > a[i].t_stop) {
		w += fabs (a[i].omega * (t - a[i].t_stop));
	}

	return (w);
}

void spotter_make_rot_matrix (double lonp, double latp, double w, double R[3][3])
{
/*	lonp, latp	Euler pole in degrees
 *	w		angular rotation in degrees
 *
 *	R		the rotation matrix
 */

	double E[3];

        GMT_geo_to_cart (latp, lonp, E, TRUE);
	make_rot_matrix_sub (E, w, R);
}

void make_rot_matrix_sub (double E[3], double w, double R[3][3])
{
/*	E	Euler pole in in cartesian coordinates
 *	w	angular rotation in degrees
 *
 *	R	the rotation matrix
 */

	double sin_w, cos_w, c, E_x, E_y, E_z, E_12c, E_13c, E_23c;

	sincosd (w, &sin_w, &cos_w);
	c = 1 - cos_w;

	E_x = E[0] * sin_w;
	E_y = E[1] * sin_w;
	E_z = E[2] * sin_w;
	E_12c = E[0] * E[1] * c;
	E_13c = E[0] * E[2] * c;
	E_23c = E[1] * E[2] * c;

	R[0][0] = E[0] * E[0] * c + cos_w;
	R[0][1] = E_12c - E_z;
	R[0][2] = E_13c + E_y;

	R[1][0] = E_12c + E_z;
	R[1][1] = E[1] * E[1] * c + cos_w;
	R[1][2] = E_23c - E_x;

	R[2][0] = E_13c - E_y;
	R[2][1] = E_23c + E_x;
	R[2][2] = E[2] * E[2] * c + cos_w;
}

void make_rot0_matrix (double lonp, double latp, double R[3][3], double E[])
{	/* This starts setting up the matrix without knowing the angle of rotation
	 * Call set_rot_angle with R, E, and omega to complete the matrix
	 * lonp, latp	Euler pole in degrees
	 *
	 *	R		the rotation matrix without terms depending on omega
	 */

        GMT_geo_to_cart (latp, lonp, E, TRUE);

	R[0][0] = E[0] * E[0];
	R[0][1] = E[0] * E[1];
	R[0][2] = E[0] * E[2];

	R[1][0] = E[0] * E[1];
	R[1][1] = E[1] * E[1];
	R[1][2] = E[1] * E[2];

	R[2][0] = E[0] * E[2];
	R[2][1] = E[1] * E[2];
	R[2][2] = E[2] * E[2];
}

void set_rot_angle (double w, double R[3][3], double E[])
{	/* Sets R using R(no_omega) and the given rotation angle w in radians */
	double sin_w, cos_w, c, E_x, E_y, E_z;

	sincos (w, &sin_w, &cos_w);
	c = 1 - cos_w;

	E_x = E[0] * sin_w;
	E_y = E[1] * sin_w;
	E_z = E[2] * sin_w;

	R[0][0] = R[0][0] * c + cos_w;
	R[0][1] = R[0][1] * c - E_z;
	R[0][2] = R[0][2] * c + E_y;

	R[1][0] = R[1][0] * c + E_z;
	R[1][1] = R[1][1] * c + cos_w;
	R[1][2] = R[1][2] * c - E_x;

	R[2][0] = R[2][0] * c - E_y;
	R[2][1] = R[2][1] * c + E_x;
	R[2][2] = R[2][2] * c + cos_w;
}

void spotter_matrix_mult (double a[3][3], double b[3][3], double c[3][3])
{	/* C = A * B */
	int i, j, k;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			c[i][j] = 0.0;
			for (k = 0; k < 3; k++) c[i][j] += a[i][k] * b[k][j];
		}
	}
}

void spotter_matrix_vect_mult (double a[3][3], double b[3], double c[3])
{	/* c = A * b */
	int i, j;

	for (i = 0; i < 3; i++) for (j = 0, c[i] = 0.0; j < 3; j++) c[i] += a[i][j] * b[j];
}

void spotter_matrix_transpose (double At[3][3], double A[3][3])
{
	/* Computes the matrix transpose */

	int i, j;
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 3; i++) {
			At[i][j] = A[j][i];
		}
	}
}

void spotter_matrix_add (double A[3][3], double B[3][3], double C[3][3])
{
	/* Computes the matrix addition */

	int i, j;
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 3; i++) {
			C[i][j] = A[i][j] + B[i][j];
		}
	}
}

void matrix_to_pole (double T[3][3], double *plon, double *plat, double *w)
{
	double T13_m_T31, T32_m_T23, T21_m_T12, L, H, tr;

	T13_m_T31 = T[0][2] - T[2][0];
	T32_m_T23 = T[2][1] - T[1][2];
	T21_m_T12 = T[1][0] - T[0][1];
	H = T32_m_T23 * T32_m_T23 + T13_m_T31 * T13_m_T31;
	L = sqrt (H + T21_m_T12 * T21_m_T12);
	H = sqrt (H);
	tr = T[0][0] + T[1][1] + T[2][2];

	*plon = atan2d (T13_m_T31, T32_m_T23);
	if (*plon < 0.0) (*plon) += 360.0;
	*plat = atan2d (T21_m_T12, H);
	*w = atan2d (L, tr - 1.0);
	if (*plat < 0.0) {	/* Make N hemisphere pole */
		*plat = -(*plat);
		*(plon) += 180.0;
		if (*plon > 360.0) *plon -=-360.0;
		*w = -(*w);
	}
}

void reverse_rotation_order (struct EULER *p, GMT_LONG n)
{	/* Simply shuffles the array from 1:n to n:1 */
	GMT_LONG i, j;
	struct EULER p_tmp;

	for (i = 0; i < n/2; i++) {
		j = n - i - 1;
		if (i != j) {
			p_tmp = p[i];
			p[i] = p[j];
			p[j] = p_tmp;
		}
	}
}

void xyw_to_struct_euler (struct EULER *p, double lon[], double lat[], double w[], GMT_LONG n, GMT_LONG stages, GMT_LONG convert)
{	/* Reload the EULER structure from the lon, lat, w arrays.
	 * stages is TRUE if we are loading stage rotations (FALSE is finite poles).
	 * convert is TRUE if we must change angles to rates or vice versa */
	GMT_LONG i;

	for (i = 0; i < n; i++) {
		p[i].lon = lon[i];
		p[i].lat = lat[i];
		p[i].duration = (stages) ? p[i].t_start - p[i].t_stop : p[i].t_start;
		p[i].omega = w[i];
		if (convert) p[i].omega /= p[i].duration;	/* Convert opening angle to opening rate */
		p[i].omega_r = p[i].omega * D2R;
		p[i].sin_lat = sind (p[i].lat);
		p[i].cos_lat = cosd (p[i].lat);
		p[i].lon_r = p[i].lon * D2R;
		p[i].lat_r = p[i].lat * D2R;
	}
}

void set_I_matrix (double R[3][3])
{	/* Simply sets R to I, the identity matrix */

	memset ((void *)R, 0, (size_t)(9 * sizeof (double)));
	R[0][0] = R[1][1] = R[2][2] = 1.0;
}

int spotter_conf_ellipse (double lon, double lat, double t, struct EULER *p, GMT_LONG np, char flag, GMT_LONG forward, double out[])
{
	/* Given time and rotation parameters, calculate uncertainty in the
	 * reconstructed point in the form of a confidence ellipse.  To follow
	 * the stuff below, it helps to realize that the covariance matrix C that
	 * is stored with each rotation R is for the rotation R which rotates a
	 * point of age t along a chain back to the hotspot.  However, in this
	 * context (the error in a reconstructed point along the chain) we are
	 * actually using the inverse rotation R^t (negative opening angle).  For
	 * that rotation, the covariance matrix is R * cov(r) * R^t.
	 * forward is TRUE if we rotate from past to now and FALSE if we
	 * rotate from now to the past (e.g., move a hotspot up the chain).
	 * 2011-02-23 PW: Multiply axes by two to get major & minor (we reported
	 * SEMI-axes previously; psxy had same problem and now expects full axes)
	 */

	GMT_LONG matrix_dim = 3L;
	GMT_LONG i, j, k, kk = 3, nrots;
	double R[3][3], x[3], y[3], M[3][3], RMt[3][3], Rt[3][3], MRt[3][3], cov[3][3], tmp[3][3], C[9];
	double z_unit_vector[3], EigenValue[3], EigenVector[9], work1[3], work2[3], x_in_plane[3], y_in_plane[3];
	double x_comp, y_comp, w;

	/* Find the unique rotation in question */

	for (i = 0, k = -1; k < 0 && i < np; i++) if (GMT_IS_ZERO (p[i].t_start - t)) k = i;
	if (k == -1) return (1);	/* Did not match finite rotation time */

	/* Make M(x), the skew-symmetric matrix needed to compute cov of rotated point */

	spotter_set_M (lon, lat, M);

	w = p[k].omega * p[k].duration;
	if (forward) w = -w;	/* Want the inverse rotation */
	spotter_make_rot_matrix (p[k].lon, p[k].lat, w, R);
	spotter_matrix_transpose (Rt, R);			/* Get the transpose of R^t */
	if (!forward) {		/* Rotate the point into the present */
		memcpy ((void *)cov, p[k].C, 9*sizeof (double));	/* The rotation's covarience matrix */
	}
	else {	/* Use inverse rotation to rotate the point from the present to past rotations */
		/* We change the sign of w so then R is actually R^t */
		/* Since we are using the inverse rotation we must first get the cov matrix of the
		   inverse rotation: cov(r^t) = R cov(r) R^t.
		   Here, R actually contains R^t so we need the original R (which we will call R^t) as well. */

		spotter_matrix_mult (p[k].C, R, tmp);			/* Calculate the cov(r) *R^t product */
		spotter_matrix_mult (Rt, tmp, cov);			/* cov(r^t) = R^t cov(r) R */
	}

	/* Calculate cov(y) = R * M^T * cov_R * M * R^T */

	spotter_matrix_mult (M, Rt, MRt);		/* Calculate the M * R^T product */
	spotter_matrix_transpose (RMt, MRt);		/* Get the transpose (M*R^T)^T = R * M^T */
	spotter_matrix_mult (cov, MRt, tmp);		/* Get C * M * R^T */
	spotter_matrix_mult (RMt, tmp, M);		/* Finally get R * M * C * M^T * R^T, store result in M */

	for (i = 0; i < 3; i++) for (j = 0; j < 3; j++) C[3*i+j] = M[i][j];	/* Reformat to 1-D format for GMT_jacobi */

	/* Get projected point y = R*x */

	GMT_geo_to_cart (lat, lon, x, TRUE);
	for (i = 0; i < 3; i++) y[i] = R[i][0] * x[0] + R[i][1] * x[1] + R[i][2] * x[2];
        GMT_cart_to_geo (&out[1], &out[0], y, TRUE);
	if (flag == 't')
		out[2] = t;
	else if (flag == 'a')
		out[2] = w;
	else
		kk = 2;

	if (GMT_jacobi (C, &matrix_dim, &matrix_dim, EigenValue, EigenVector, work1, work2, &nrots)) {	/* Solve eigen-system C = EigenVector * EigenValue * EigenVector^T */
		fprintf (stderr,"libspotter: Warning: Eigenvalue routine failed to converge in 50 sweeps.\n");
	}

	z_unit_vector[0] = z_unit_vector[1] = 0.0;	z_unit_vector[2] = 1.0;	/* z unit vector */
	GMT_cross3v (z_unit_vector, y, x_in_plane);	/* Local x-axis in plane normal to mean pole */
	GMT_cross3v (y, x_in_plane, y_in_plane);	/* Local y-axis in plane normal to mean pole */
	x_comp = GMT_dot3v (EigenVector, x_in_plane);	/* x-component of major axis in tangent plane */
	y_comp = GMT_dot3v (EigenVector, y_in_plane);	/* y-component of major axis in tangent plane */
	out[kk] = fmod (360.0 + (90.0 - atan2d (y_comp, x_comp)), 360.0);	/* Azimuth of major axis */
	if (out[kk] > 180.0) out[kk] -= 180.0;
	out[++kk] = 2.0 * sqrt (EigenValue[0]) * EQ_RAD * SQRT_CHI2;	/* Full major axis (not semi) */
	out[++kk] = 2.0 * sqrt (EigenValue[1]) * EQ_RAD * SQRT_CHI2;	/* Full minor axis (not semi) */

	return (0);
}

void spotter_set_M (double lon, double lat, double M[3][3])
{	/* Just initializes the M(x), the skew-symmetric matrix needed to compute cov of rotated point */
	double x[3];
        GMT_geo_to_cart (lat, lon, x, TRUE);	/* Get Cartesian vector for this point */
	M[0][0] = M[1][1] = M[2][2] = 0.0;
	M[0][1] = -x[2];
	M[0][2] = x[1];
	M[1][0] = x[2];
	M[1][2] = -x[0];
	M[2][0] = -x[1];
	M[2][1] = x[0];
}

void spotter_covar_to_record (struct EULER *e, double K[])
{
	/* Translates an Euler covariance matrix to the 9 values needed for printout
	 * covariance matrix is stored as [k_hat a b c d e f g df] */
	
	int k;
	K[0] = e->k_hat;
	K[7] = e->g;
	K[8] = e->df;
	K[1] = e->C[0][0];
	K[2] = e->C[0][1];
	K[4] = e->C[0][2];
	K[3] = e->C[1][1];
	K[5] = e->C[1][2];
	K[6] = e->C[2][2];
	for (k = 1; k < 7; k++) K[k] *= (e->k_hat / e->g);
}

void record_to_covar (struct EULER *e, double K[])
{
	/* Translates the 9 values read from plate motion file [k_hat a b c d e f g df]
	 * into the Euler covariance matrix */
	
	int k, j;
	e->has_cov = TRUE;
	e->k_hat   = K[0];
	e->g       = K[7];
	e->df	   = K[8];
	e->C[0][0] = K[1];
	e->C[0][1] = e->C[1][0] = K[2];
	e->C[0][2] = e->C[2][0] = K[4];
	e->C[1][1] = K[3];
	e->C[1][2] = e->C[2][1] = K[5];
	e->C[2][2] = K[6];
	for (k = 0; k < 3; k++) for (j = 0; j < 3; j++) e->C[k][j] *= (e->g / e->k_hat);
}

void spotter_cov_of_inverse (struct EULER *e, double Ct[3][3])
{	/* If A and cov(u) is a rotation and its covariance matrix and
	 * let A' and cov(v) be the inverse rotation and its covariacne matrix,
	 * then cov(v) = A*cov(u)*A' */
	
	double A[3][3], At[3][3], tmp[3][3];
	
	spotter_make_rot_matrix (e->lon, e->lat, e->omega, A);
	spotter_matrix_transpose (At, A);	/* Get A' */
	spotter_matrix_mult (e->C, At, tmp);	/* Calculate the cov(u)*A' product */
	spotter_matrix_mult (A, tmp, Ct);	/* Calculate the cov(v) = A*cov(u)*A' product */
}
