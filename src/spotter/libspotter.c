/*--------------------------------------------------------------------
 *	$Id: libspotter.c,v 1.4 2001-10-19 20:48:35 pwessel Exp $
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
 * SPOTTER: functions for moving points along small circles on a sphere.
 *
 * Paul Wessel, University of Hawaii
 * June 09, 1999
 * Version 1.0
 *
 * The user-callable functions in this library are:
 *
 * spotter_init			: Load stage poles from file
 * spotter_backtrack		: Trace track from seamount to hotspot
 * spotter_forthtrack		: Trace track from hotspot to seamount
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
 */

#include "spotter.h"

/* Internal functions */

void spotter_forward_poles (struct EULER p[], int ns);
void spotter_rotate_fwd (double lon, double lat, double *tlon, double *tlat, struct EULER *p);
void spotter_rotate_inv (double *lon, double *lat, double tlon, double tlat, struct EULER *p);
void matrix_to_pole (double T[3][3], double *plon, double *plat, double *w);
void matrix_transpose (double At[3][3], double A[3][3]);
void matrix_mult (double a[3][3], double b[3][3], double c[3][3]);
void make_rot_matrix (double lonp, double latp, double w, double R[3][3]);
void finite_to_stages (struct EULER p[], int n, int flag);
void stages_to_finite (struct EULER p[], int n, int flag);

int spotter_init (char *file, struct EULER **p, int flowline, int finite, double *t_max)
{
	/* file;	Name of file with backward stage poles */
	/* p;		Pointer to stage pole array */
	/* flowline;	TRUE if flowlines rather than hotspot-tracks are needed */
	/* finite;	TRUE for finite (total construction poles) files [Default is stage poles] */
	/* t_max;	Extend earliest stage pole back to this age */
	FILE *fp;
	struct EULER *e;
	char  buffer[BUFSIZ];
	int n, nf, i = 0, n_alloc = GMT_SMALL_CHUNK;
	double x, y, last_t;

	e = (struct EULER *) GMT_memory (VNULL, n_alloc, sizeof (struct EULER), "libspotter");

	if ((fp = fopen (file, "r")) == NULL) {
		fprintf (stderr, "EULER: Cannot open stage pole file: %s\n", file);
		exit (EXIT_FAILURE);
	}

	last_t = (finite) ? 0.0 : DBL_MAX;
	while (fgets (buffer, 512, fp) != NULL) { /* Expects lon lat t0 t1 ccw-angle */
		if (buffer[0] == '#' || buffer[0] == '\n') continue;

		nf = sscanf (buffer, "%lf %lf %lf %lf %lf", &e[i].lon, &e[i].lat, &e[i].t_start, &e[i].t_stop, &e[i].omega);

		if (finite && nf == 4) e[i].omega = e[i].t_stop, e[i].t_stop = 0.0;	/* Only got 4 columns */
		
		if (e[i].t_stop >= e[i].t_start) {
			fprintf (stderr, "libspotter: ERROR: Stage rotation %d has start time younger than stop time\n", i);
			exit (EXIT_FAILURE);
		}
		e[i].duration = e[i].t_start - e[i].t_stop;
		if (finite) {
			if (e[i].t_start < last_t) {
				fprintf (stderr, "libspotter: ERROR: Finite rotations must go from youngest to oldest\n");
				exit (EXIT_FAILURE);
			}
			last_t = e[i].t_start;
		}
		else {
			if (e[i].t_stop > last_t) {
				fprintf (stderr, "libspotter: ERROR: Stage rotations must go from oldest to youngest\n");
				exit (EXIT_FAILURE);
			}
			last_t = e[i].t_stop;
			e[i].omega /= e[i].duration;	/* For finite, omega is opening angle, not rate; this will change in finite_to_stages */
		}
		
		e[i].omega_r = e[i].omega * D2R;
		e[i].sin_lat = sin (e[i].lat * D2R);
		e[i].cos_lat = cos (e[i].lat * D2R);
		x = e[i].lon * D2R;	y = e[i].lat * D2R;
		e[i].lon_r = x;		e[i].lat_r = y;
		i++;
		if (i == n_alloc) {
			n_alloc += GMT_SMALL_CHUNK;
			e = (struct EULER *) GMT_memory ((void *)e, n_alloc, sizeof (struct EULER), "libspotter");
		}
	}
	fclose (fp);
	
	n = i;

	if (finite) finite_to_stages (e, n, finite-1);	/* Convert finite poles to backward stage poles */
	
	/* Extend oldest stage pole back to t_max Ma */

	if ((*t_max) > 0.0 && e[0].t_start < (*t_max)) {
		fprintf (stderr, "Extending oldest stage pole back to %lg Ma\n", (*t_max));

		e[0].t_start = (*t_max);
		e[0].duration = e[0].t_start - e[0].t_stop;
	}
	else
		(*t_max) = e[0].t_start;

	e = (struct EULER *) GMT_memory ((void *)e, n, sizeof (struct EULER), "libspotter");
	if (flowline) spotter_forward_poles (e, n);
	*p = e;

	return (n);
}

void spotter_forward_poles (struct EULER p[], int ns)
{
	int i, j;
	double d_lon, tlon, tlat;

	for (i = ns-1; i > 0; i--) {	/* From current to previous stages */
		for (j = 0; j < i; j++) {	/* Rotate the older stage poles */
			d_lon = p[i].omega_r * p[i].duration;
			spotter_rotate_fwd (p[j].lon_r, p[j].lat_r, &tlon, &tlat, &p[i]);
			tlon += d_lon;
			spotter_rotate_inv (&p[j].lon_r, &p[j].lat_r, tlon, tlat, &p[i]);
		}
	}
	for (i = 0; i < ns; i++) {
		p[i].lon = p[i].lon_r * R2D;
		p[i].lat = p[i].lat_r * R2D;
		p[i].sin_lat = sin (p[i].lat_r);
		p[i].cos_lat = cos (p[i].lat_r);
		p[i].omega = -p[i].omega;
		p[i].omega_r = -p[i].omega_r;
	}
}

/* spotter_backtrack: Given a seamount location and age, trace the
 *	hotspot-track between this seamount and a seamount of 
 *	age t_zero.  For t_zero = 0 this means the hotspot
 */

int spotter_backtrack (double xp[], double yp[], double tp[], int np, struct EULER p[], int ns, double d_km, double t_zero, BOOLEAN do_time, double **c)
/* xp, yp;	Points, in RADIANS */
/* tp;		Age of feature in m.y. */
/* np;		# of points */
/* p;		Stage poles */
/* ns;		# of stage poles */
/* d_km;	Create track point every d_km km.  If == -1.0, return bend points only */
/* t_zero;	Backtrack up to this age */
/* do_time;	TRUE if we want to interpolate and return time along track, 2 if we just want stage # */
/* **c;		Pointer to return trac vector */
{
	int i, j, k, kk = 0, start_k, nd = 1, nn, n_alloc = 2 * GMT_CHUNK;
	BOOLEAN path, bend;
	double t, tt, dt, d_lon, tlon, dd, i_km, xnew, xx, yy, *track;
	double s_lat, c_lat, s_lon, c_lon, cc, ss, cs, i_nd;

	bend = (d_km <= (SMALL - 1.0));
	path = (bend || d_km > SMALL);

	if (path) {
		track = (double *) GMT_memory (VNULL, n_alloc, sizeof (double), "libspotter");
		i_km = EQ_RAD / d_km;
	}

	if (p[ns-1].t_stop > t_zero) t_zero = p[ns-1].t_stop;	/* In case we dont go all the way to zero */
	
	for (i = 0; i < np; i++) {

		if (path) {
			start_k = kk++;
			if (kk == n_alloc) {
				n_alloc += BIG_CHUNK;
				track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
			}
		}
		nn = 0;
			
		t = tp[i];
		while (t > t_zero) {	/* As long as we're not back at zero age */

			j = 0;
			while (j < ns && t <= p[j].t_stop) j++;	/* Find first applicable stage pole */
			if (j == ns) {
				fprintf (stderr, "libspotter: (spotter_backtrack) Ran out of stage poles for t = %lg\n", t);
				exit (EXIT_FAILURE);
			}
			dt = MIN (p[j].duration, t - MAX(p[j].t_stop, t_zero));
			d_lon = p[j].omega_r * dt;

			/* spotter_rotate_fwd (xp[i], yp[i], &tlon, &tlat, &p[j]); */

			xnew = xp[i] - p[j].lon_r;
			sincos (yp[i], &s_lat, &c_lat);
			sincos (xnew, &s_lon, &c_lon);
			cc = c_lat * c_lon;
			tlon = d_atan2 (c_lat * s_lon, p[j].sin_lat * cc - p[j].cos_lat * s_lat);
			s_lat = p[j].sin_lat * s_lat + p[j].cos_lat * cc;
			c_lat = sqrt (1.0 - s_lat * s_lat);
			ss = p[j].sin_lat * s_lat;
			cs = p[j].cos_lat * s_lat;

			if (path) {
				if (!bend) {
					nd = (int) ceil ((fabs (d_lon) * c_lat) * i_km);
					i_nd = 1.0 / nd;
					dd = d_lon * i_nd;
					tt = dt * i_nd;
				}
				track[kk++] = xp[i];
				if (kk == n_alloc) {
					n_alloc += BIG_CHUNK;
					track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
				}
				track[kk++] = yp[i];
				if (kk == n_alloc) {
					n_alloc += BIG_CHUNK;
					track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
				}
				if (do_time) {
					track[kk++] = (do_time == 2) ? (double)(ns - j) : t;
					if (kk == n_alloc) {
						n_alloc += BIG_CHUNK;
						track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
					}
				}
				for (k = 1; k < nd; k++) {
					/* spotter_rotate_inv (&xx, &yy, tlon + k * dd, tlat, &p[j]); */

					xnew = tlon + k * dd;
					sincos (xnew, &s_lon, &c_lon);
					cc = c_lat * c_lon;
					yy = d_asin (ss - p[j].cos_lat * cc);
					xx = p[j].lon_r + d_atan2 (c_lat * s_lon, p[j].sin_lat * cc + cs);

					if (xx < 0.0) xx += TWO_PI;
					if (xx >= TWO_PI) xx -= TWO_PI;
					track[kk++] = xx;
					if (kk == n_alloc) {
						n_alloc += BIG_CHUNK;
						track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
					}
					track[kk++] = yy;
					if (kk == n_alloc) {
						n_alloc += BIG_CHUNK;
						track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
					}
					if (do_time) {
						track[kk++] = (do_time == 2) ? (double)(ns - j) : t - k * tt;
						if (kk == n_alloc) {
							n_alloc += BIG_CHUNK;
							track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
						}
					}
				}
				nn += nd;
			}
			/* spotter_rotate_inv (&xp[i], &yp[i], tlon + d_lon, tlat, &p[j]); */
			xnew = tlon + d_lon;
			sincos (xnew, &s_lon, &c_lon);
			cc = c_lat * c_lon;
			yp[i] = d_asin (ss - p[j].cos_lat * cc);
			xp[i] = p[j].lon_r + d_atan2 (c_lat * s_lon, p[j].sin_lat * cc + cs);

			if (xp[i] < 0.0) xp[i] += TWO_PI;
			if (xp[i] >= TWO_PI) xp[i] -= TWO_PI;
			t -= dt;
		}
		if (path) {
			track[kk++] = xp[i];
			if (kk == n_alloc) {
				n_alloc += BIG_CHUNK;
				track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
			}
			track[kk++] = yp[i];
			if (kk == n_alloc) {
				n_alloc += BIG_CHUNK;
				track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
			}
			if (do_time) {
				track[kk++] = (do_time == 2) ? (double)(ns - j) : t;
				if (kk == n_alloc) {
					n_alloc += BIG_CHUNK;
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

/* spotter_forthtrack: Given a hotspot location and final age, trace the
 *	hotspot-track between the seamount created at t_zero and a
 *	seamount of age tp.  For t_zero = 0 this means from the hotspot.
 */

int spotter_forthtrack (double xp[], double yp[], double tp[], int np, struct EULER p[], int ns, double d_km, double t_zero, BOOLEAN do_time, double **c)
/* xp, yp;	Points, in RADIANS */
/* tp;		Age of feature in m.y. */
/* np;		# of points */
/* p;		Stage poles */
/* ns;		# of stage poles */
/* d_km;	Create track point every d_km km.  If == -1.0, return bend points only */
/* t_zero;	Foretrack from this age forward */
/* do_time;	TRUE if we want to interpolate and return time along track */
/* c;		Pointer to return trac vector */
{
	int i, j, k, kk = 0, start_k, nd = 1, nn, n_alloc = 2 * GMT_CHUNK;
	BOOLEAN path, bend;
	double t, tt, dt, d_lon, tlon, dd, i_km, xnew, xx, yy, *track;
	double s_lat, c_lat, s_lon, c_lon, cc, ss, cs, i_nd;

	bend = (d_km <= (SMALL - 1.0));
	path = (bend || d_km > SMALL);

	if (path) {
		track = (double *) GMT_memory (VNULL, n_alloc, sizeof (double), "libspotter");
		i_km = EQ_RAD / d_km;
	}

	if (p[ns-1].t_stop > t_zero) t_zero = p[ns-1].t_stop;	/* In case we dont go all the way to zero */

	for (i = 0; i < np; i++) {

		if (path) {
			start_k = kk++;
			if (kk == n_alloc) {
				n_alloc += BIG_CHUNK;
				track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
			}
		}
		nn = 0;

		t = t_zero;
		while (t < tp[i]) {	/* As long as we're not back at zero age */

			j = 0;
			while (j < ns && t < p[j].t_stop) j++;	/* Find first applicable stage pole */
			if (j == ns) {
				fprintf (stderr, "libspotter: (spotter_forthtrack) Ran out of stage poles for t = %lg\n", t);
				exit (EXIT_FAILURE);
			}
			dt = MIN (tp[i], p[j].t_start) - t;	/* Time interval to rotate */
			d_lon = p[j].omega_r * dt;		/* Rotation angle (radians) */
			/* spotter_rotate_fwd (xp[i], yp[i], &tlon, &tlat, &p[j]); */

			xnew = xp[i] - p[j].lon_r;
			sincos (yp[i], &s_lat, &c_lat);
			sincos (xnew, &s_lon, &c_lon);
			cc = c_lat * c_lon;
			tlon = d_atan2 (c_lat * s_lon, p[j].sin_lat * cc - p[j].cos_lat * s_lat);
			s_lat = p[j].sin_lat * s_lat + p[j].cos_lat * cc;
			c_lat = sqrt (1.0 - s_lat * s_lat);
			ss = p[j].sin_lat * s_lat;
			cs = p[j].cos_lat * s_lat;

			if (path) {
				if (!bend) {
					nd = (int) ceil ((fabs (d_lon) * c_lat) * i_km);
					i_nd = 1.0 / nd;
					dd = d_lon * i_nd;
					tt = dt * i_nd;
				}
				track[kk++] = xp[i];
				if (kk == n_alloc) {
					n_alloc += BIG_CHUNK;
					track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
				}
				track[kk++] = yp[i];
				if (kk == n_alloc) {
					n_alloc += BIG_CHUNK;
					track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
				}
				if (do_time) {
					track[kk++] = (do_time == 2) ? (double)(ns - j) : t;
					if (kk == n_alloc) {
						n_alloc += BIG_CHUNK;
						track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
					}
				}
				for (k = 1; k < nd; k++) {
					/* spotter_rotate_inv (&xx, &yy, tlon - k * dd, tlat, &p[j]); */
					xnew = tlon - k * dd;
					sincos (xnew, &s_lon, &c_lon);
					cc = c_lat * c_lon;
					yy = d_asin (ss - p[j].cos_lat * cc);
					xx = p[j].lon_r + d_atan2 (c_lat * s_lon, p[j].sin_lat * cc + cs);

					if (xx < 0.0) xx += TWO_PI;
					if (xx >= TWO_PI) xx -= TWO_PI;
					track[kk++] = xx;
					if (kk == n_alloc) {
						n_alloc += BIG_CHUNK;
						track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
					}
					track[kk++] = yy;
					if (kk == n_alloc) {
						n_alloc += BIG_CHUNK;
						track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
					}
					if (do_time) {
						track[kk++] = (do_time == 2) ? (double)(ns - j) : t + k * tt;
						if (kk == n_alloc) {
							n_alloc += BIG_CHUNK;
							track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
						}
					}
				}
				nn += nd;
			}
			/* spotter_rotate_inv (&xp[i], &yp[i], tlon - d_lon, tlat, &p[j]); */
			xnew = tlon - d_lon;
			sincos (xnew, &s_lon, &c_lon);
			cc = c_lat * c_lon;
			yp[i] = d_asin (ss - p[j].cos_lat * cc);
			xp[i] = p[j].lon_r + d_atan2 (c_lat * s_lon, p[j].sin_lat * cc + cs);

			if (xp[i] < 0.0) xp[i] += TWO_PI;
			if (xp[i] >= TWO_PI) xp[i] -= TWO_PI;
			t += dt;
		}
		if (path) {
			track[kk++] = xp[i];
			if (kk == n_alloc) {
				n_alloc += BIG_CHUNK;
				track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
			}
			track[kk++] = yp[i];
			if (kk == n_alloc) {
				n_alloc += BIG_CHUNK;
				track = (double *) GMT_memory ((void *)track, (size_t)n_alloc, sizeof (double), "libspotter");
			}
			if (do_time) {
				track[kk++] = (do_time == 2) ? (double)(ns - j) : t;
				if (kk == n_alloc) {
					n_alloc += BIG_CHUNK;
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

void spotter_rotate_fwd (double lon, double lat, double *tlon, double *tlat, struct EULER *p)
{
	/* Given the pole position in p, oblique coordinates
	 * are computed from geographical coordinates assuming a spherical earth.
	 * All values in RADIANS
	 */
	 
	double dlon, cc, test, s_lat, c_lat, c_lon, s_lon;
	 
	dlon = lon - p->lon_r;
	sincos (lat, &s_lat, &c_lat);
	sincos (dlon, &s_lon, &c_lon);
	cc = c_lat * c_lon;
	test = p->sin_lat * s_lat + p->cos_lat * cc;
	*tlat = d_asin (test);
	*tlon = d_atan2 (c_lat * s_lon, p->sin_lat * cc - p->cos_lat * s_lat);
}
	 
void spotter_rotate_inv (double *lon, double *lat, double tlon, double tlat, struct EULER *p)
{
	/* Given the pole position in project_info, geographical coordinates 
	 * are computed from oblique coordinates assuming a spherical earth.
	 * All values in RADIANS
	 */
	 
	double dlon, test, s_lat, c_lat, c_lon, s_lon, cc;
	 
	dlon = tlon;
	sincos (tlat, &s_lat, &c_lat);
	sincos (dlon, &s_lon, &c_lon);
	cc = c_lat * c_lon;
	test = p->sin_lat * s_lat - p->cos_lat * cc;
	*lat = d_asin (test);
	*lon = p->lon_r + d_atan2 (c_lat * s_lon, p->sin_lat * cc + p->cos_lat * s_lat);
}

/* Converts a set of total reconstruction poles to stage poles
 *
 * Based partly on Cox and Hart, 1986
 */

void finite_to_stages (struct EULER p[], int n, int flag)
{
	int i, j;
	double *elon, *elat, *ew, t_old;
	double R_new[3][3], R_old[3][3], R_stage[3][3];
	struct EULER e_tmp;

	/* Expects total reconstruction models to have youngest poles first */
	
	elon = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "libspotter");
	elat = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "libspotter");
	ew   = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "libspotter");
	
	memset ((void *)R_new, 0, (size_t)(9 * sizeof (double)));
	R_new[0][0] = R_new[1][1] = R_new[2][2] = 1.0;
	
	t_old = 0.0;
	for (i = 0; i < n; i++) {
		make_rot_matrix (p[i].lon, p[i].lat, p[i].omega, R_old);
		matrix_mult (R_new, R_old, R_stage);
		matrix_to_pole (R_stage, &elon[i], &elat[i], &ew[i]);
		if (elon[i] > 180.0) elon[i] -= 360.0;
		matrix_transpose (R_new, R_old);
		p[i].t_stop = t_old;
		t_old = p[i].t_start;
	}
	
	/* Time to put back */
	
	for (i = 0; i < n; i++) {
		p[i].lon = elon[i];
		p[i].lat = elat[i];
		p[i].duration = p[i].t_start - p[i].t_stop;
		p[i].omega = ew[i];
		if (flag == 0) p[i].omega /= p[i].duration;	/* get rates */
		p[i].omega_r = p[i].omega * D2R;
		p[i].sin_lat = sin (p[i].lat * D2R);
		p[i].cos_lat = cos (p[i].lat * D2R);
		p[i].lon_r = p[i].lon * D2R;	
		p[i].lat_r = p[i].lat * D2R;
	}

	GMT_free ((void *)elon);
	GMT_free ((void *)elat);
	GMT_free ((void *)ew);
	
	/* Flip order */
	
	for (i = 0; i < n/2; i++) {
		j = n - i - 1;
		if (i != j) {
			e_tmp = p[i];
			p[i] = p[j];
			p[j] = e_tmp;
		}
	}
}

void stages_to_finite (struct EULER p[], int n, int flag)
{
	int i, j;
	double *elon, *elat, *ew;
	double R_new[3][3], R_old[3][3], R_stage[3][3];
	struct EULER e_tmp;

	/* Expects stage pole models to have oldest poles first, so we must flip order */
	
	for (i = 0; i < n/2; i++) {
		j = n - i - 1;
		if (i != j) {
			e_tmp = p[i];
			p[i] = p[j];
			p[j] = e_tmp;
		}
	}
	
	elon = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "libspotter");
	elat = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "libspotter");
	ew   = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "libspotter");
	
	memset ((void *)R_old, 0, (size_t)(9 * sizeof (double)));
	R_old[0][0] = R_old[1][1] = R_old[2][2] = 1.0;
	
	for (i = 0; i < n; i++) {
		if (flag) p[i].omega *= p[i].duration;	/* Because spotter_init converted angles to rates */
		make_rot_matrix (p[i].lon, p[i].lat, -p[i].omega, R_stage);
		matrix_mult (R_stage, R_old, R_new);
		memcpy ((void *)R_old, (void *)R_new, (size_t)(9 * sizeof (double)));
		matrix_to_pole (R_new, &elon[i], &elat[i], &ew[i]);
		if (elon[i] > 180.0) elon[i] -= 360.0;
		p[i].lon = elon[i];	
		p[i].lat = elat[i];
		p[i].duration = p[i].t_start;
		p[i].omega = -ew[i];
		p[i].omega_r = p[i].omega * D2R;
		p[i].sin_lat = sin (p[i].lat * D2R);
		p[i].cos_lat = cos (p[i].lat * D2R);
		p[i].lon_r = p[i].lon * D2R;	
		p[i].lat_r = p[i].lat * D2R;
	}
	
	GMT_free ((void *)elon);
	GMT_free ((void *)elat);
	GMT_free ((void *)ew);
}

void make_rot_matrix (double lonp, double latp, double w, double R[3][3])
{
/*	lonp, latp	Euler pole in degrees
 *	w		angular rotation in degrees
 *
 *	R		the rotation matrix
 */

	double E[3], sin_w, cos_w, c, E_x, E_y, E_z, E_12c, E_13c, E_23c;
	
	sincos (w * D2R, &sin_w, &cos_w);
        GMT_geo_to_cart (&latp, &lonp, E, TRUE);
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

void matrix_mult (double a[3][3], double b[3][3], double c[3][3])
{	/* C = A * B */
	int i, j, k;
	
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			c[i][j] = 0.0;
			for (k = 0; k < 3; k++) c[i][j] += a[i][k] * b[k][j];
		}
	}
}

void matrix_transpose (double At[3][3], double A[3][3])
{
	/* Computes the matrix transpose */

	int i, j;
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 3; i++) {
			At[i][j] = A[j][i];
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

	*plon = atan2 (T13_m_T31, T32_m_T23) * R2D;
	if (*plon < 0.0) (*plon) += 360.0;
	*plat = atan2 (T21_m_T12, H) * R2D;
	*w = atan2 (L, (tr - 1.0)) * R2D;
	if (*plat < 0.0) {	/* Make N hemisphere pole */
		*plat = -(*plat);
		*(plon) += 180.0;
		if (*plon > 360.0) *plon -=-360.0;
		*w = -(*w);
	}
}
