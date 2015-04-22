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
 * SPOTTER: functions for moving points along small circles on a sphere.
 *
 * Paul Wessel, University of Hawaii
 * July 20, 2010
 * Version 1.2 for GMT 5
 *
 * The user-callable functions in this library are:
 *
 * spotter_init			: Load stage poles from file
 * spotter_backtrack		: Trace track from seamount to hotspot
 * spotter_forthtrack		: Trace track from hotspot to seamount
 * spotter_total_to_stages	: Convert finite rotations to stage poles
 * spotter_stages_to_total	: Convert stage poles to finite rotations
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
 * flowline = true when calling spotter_init and then:
 *	1. Draw FROM a hotspot TO a seamount: Use spotter_backtrack
 *	2. Draw FROM a seamount TO a hotspot (and beyond): Use spotter_forthtrack
 *
 * All coordinates herein are assumed to be GEOCENTRIC.  The main programs are
 * respondible for coverting to/from geodetic data coordinates.  Rotation pole
 * latitudes are usually implied to be geocentric.
 */

#include "spotter.h"

void GMT_get_point_from_r_az (struct GMT_CTRL *GMT, double lon0, double lat0, double r, double azim, double *lon1, double *lat1);

#define SPOTTER_N_STEPS		360
#define SPOTTER_DEL_STEP	(TWO_PI/SPOTTER_N_STEPS)
#define SPOTTER_N_FINE_STEPS	36000
#define SPOTTER_N_GRID		100
#define SPOTTER_D_CUT		1.0e-6

void spotter_rot_usage (struct GMTAPI_CTRL *API, char option)
{
	GMT_Message (API, GMT_TIME_NONE, "\t-%c Specify file with the rotations to be used (see man page for format).\n", option);
	GMT_Message (API, GMT_TIME_NONE, "\t   Prepend + if you want to invert the finite rotations prior to use.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternative 1: Give a single rotation as plon/plat/prot.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternative 2: Give two plate IDs separated by a hyphen (e.g., PAC-MBL)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to extract that rotation from the GPlates rotation database (if installed).\n");
}

/* Sort functions used to order the rotations */

int spotter_comp_stage (const void *p_1, const void *p_2)
{
	/* Returns -1 if rotation pointed to by p_1 is older that point_2,
	   +1 if the reverse it true, and 0 if they are equal
	*/
	const struct EULER *point_1 = p_1, *point_2 = p_2;

	if (point_1->t_start > point_2->t_start) return (-1);
	if (point_1->t_start < point_2->t_start) return (+1);
	return (0);
}

int spotter_comp_total (const void *p_1, const void *p_2)
{
	/* Returns -1 if rotation pointed to by p_1 is older that point_2,
	   +1 if the reverse it true, and 0 if they are equal
	*/
	const struct EULER *point_1 = p_1, *point_2 = p_2;

	if (point_1->t_start < point_2->t_start) return (-1);
	if (point_1->t_start > point_2->t_start) return (1);
	return (0);
}

void spotter_matrix_to_pole (struct GMT_CTRL *GMT, double T[3][3], double *plon, double *plat, double *w)
{	/* Based on Cox and Hart, 1986 */
	double T13_m_T31, T32_m_T23, T21_m_T12, L, H, tr;
	GMT_UNUSED(GMT);

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

void make_rot0_matrix (struct GMT_CTRL *GMT, double lonp, double latp, double R[3][3], double E[])
{	/* Based on Cox and Hart, 1986 */
	/* This starts setting up the matrix without knowing the angle of rotation
	 * Call set_rot_angle with R, E, and omega to complete the matrix
	 * lonp, latp	Euler pole in degrees
	 *
	 *	R		the 3x3 rotation matrix without terms depending on omega
	 */

        GMT_geo_to_cart (GMT, latp, lonp, E, true);

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

void reverse_rotation_order (struct EULER *p, unsigned int n)
{	/* Simply reverses the array from 1:n to n:1 */
	unsigned int i, j;
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

void xyw_to_struct_euler (struct EULER *p, double lon[], double lat[], double w[], unsigned int n, unsigned int stages, bool convert)
{	/* Reload the EULER structure from the lon, lat, w arrays.
	 * stages is true if we are loading stage rotations (false is finite poles).
	 * convert is true if we must change angles to rates or vice versa */
	unsigned int i;

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

	GMT_memset (R, 9, double);
	R[0][0] = R[1][1] = R[2][2] = 1.0;
}

bool must_do_track (int sideA[], int sideB[]) {
	int dx, dy;
	/* First check if any of the two points are inside the box */
	if (sideA[0] == 0 && sideA[1] == 0) return (true);
	if (sideB[0] == 0 && sideB[1] == 0) return (true);
	/* Now check if the two points may cut a corner */
	dx = abs (sideA[0] - sideB[0]);
	dy = abs (sideA[1] - sideB[1]);
	if (dx && dy) return (true);
	if (dx == 2 || dy == 2) return (true);	/* Could cut across the box */
	return (false);
}

void set_inout_sides (double x, double y, double wesn[], int sideXY[2]) {
	/* Given the rectangular region in wesn, return -1, 0, +1 for
	 * x and y if the point is left/below (-1) in (0), or right/above (+1).
	 * 
	 */
	 
	if (y < wesn[YLO])
		sideXY[1] = -1;
	else if (y > wesn[YHI])
		sideXY[1] = +1;
	else
		sideXY[1] = 0;
	while ((x + TWO_PI) < wesn[XHI]) x += TWO_PI;
	while ((x - TWO_PI) > wesn[XLO]) x -= TWO_PI;
	if (x < wesn[XLO])
		sideXY[0] = -1;
	else if (x > wesn[XHI])
		sideXY[0] = +1;
	else
		sideXY[0] = 0;
}

void spotter_covar_to_record (struct GMT_CTRL *GMT, struct EULER *e, double K[])
{
	/* Translates an Euler covariance matrix to the 9 values needed for printout
	 * covariance matrix is stored as [k_hat a b c d e f g df] */
	
	unsigned int k;
	GMT_UNUSED(GMT);
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
	
	unsigned int k, j;
	e->has_cov = true;
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

void spotter_set_M (struct GMT_CTRL *GMT, double lon, double lat, double M[3][3])
{	/* Just initializes the M(x), the skew-symmetric matrix needed to compute cov of rotated point */
	double x[3];
        GMT_geo_to_cart (GMT, lat, lon, x, true);	/* Get Cartesian vector for this point */
	M[0][0] = M[1][1] = M[2][2] = 0.0;
	M[0][1] = -x[2];
	M[0][2] = x[1];
	M[1][0] = x[2];
	M[1][2] = -x[0];
	M[2][0] = -x[1];
	M[2][1] = x[0];
}

void spotter_cov_of_inverse (struct GMT_CTRL *GMT, struct EULER *e, double Ct[3][3])
{	/* If A and cov(u) is a rotation and its covariance matrix and
	 * let A' and cov(v) be the inverse rotation and its covariacne matrix,
	 * then cov(v) = A*cov(u)*A' */
	
	double A[3][3], At[3][3], tmp[3][3];
	
	GMT_make_rot_matrix (GMT, e->lon, e->lat, e->omega, A);
	spotter_matrix_transpose (GMT, At, A);	/* Get A' */
	spotter_matrix_mult (GMT, e->C, At, tmp);	/* Calculate the cov(u)*A' product */
	spotter_matrix_mult (GMT, A, tmp, Ct);	/* Calculate the cov(v) = A*cov(u)*A' product */
}

/* Converts a set of total reconstruction poles to forward stage poles for flowlines
 *
 * Based partly on Cox and Hart, 1986
 */

void spotter_total_to_fwstages (struct GMT_CTRL *GMT, struct EULER p[], unsigned int n, bool finite_rates, bool stage_rates)
{
	/* Convert finite rotations to forward stage rotations for flowlines */
	/* p[]		: Array of structure elements with rotation parameters
	 * n		: Number of rotations
	 * finite_rates	: true if finite rotations given in degree/my [else we have opening angle]
	 * stage_rates	: true if stage rotations should be returned in degree/my [else we return opening angle]
	 */
	 
	unsigned int i;
	double *elon = NULL, *elat = NULL, *ew = NULL, t_old;
	double R_young[3][3], R_old[3][3], R_stage[3][3];

	/* Expects total reconstruction models to have youngest poles first */

	elon = GMT_memory (GMT, NULL, n, double);
	elat = GMT_memory (GMT, NULL, n, double);
	ew   = GMT_memory (GMT, NULL, n, double);

	set_I_matrix (R_young);		/* The first time, R_young is simply I */

	/* First forward stage pole is the youngest total reconstruction pole */

	t_old = 0.0;
	for (i = 0; i < n; i++) {
		if (finite_rates) p[i].omega *= p[i].duration;			/* Convert opening rate to opening angle */
		GMT_make_rot_matrix (GMT, p[i].lon, p[i].lat, -p[i].omega, R_old);	/* Make rotation matrix from rotation parameters, take transpose by passing -omega */
		spotter_matrix_mult (GMT, R_young, R_old, R_stage);			/* This is R_stage = R_young * R_old^t */
		spotter_matrix_to_pole (GMT, R_stage, &elon[i], &elat[i], &ew[i]);	/* Get rotation parameters from matrix */
		if (elon[i] > 180.0) elon[i] -= 360.0;				/* Adjust lon */
		spotter_matrix_transpose (GMT, R_young, R_old);			/* Set R_young = (R_old^t)^t = R_old */
		p[i].t_stop = t_old;
		t_old = p[i].t_start;
	}

	/* Repopulate the EULER structure given the rotation parameters */

	xyw_to_struct_euler (p, elon, elat, ew, n, true, stage_rates);

	GMT_free (GMT, elon);
	GMT_free (GMT, elat);
	GMT_free (GMT, ew);

	/* Flip order since stages go from oldest to youngest */

	reverse_rotation_order (p, n);	/* Flip order since stages go from oldest to youngest */
}

bool spotter_GPlates_pair (char *file)
{	/* Check if given file is actually a GPlates plate pair */
	unsigned int i;
	char A[GMT_LEN64] = {""}, B[GMT_LEN64] = {""};
	if (strlen (file) > GMT_LEN64) return (false);	/* Cannot be two pairs of tags */
	if (sscanf (file, "%[^-]-%s", A, B) != 2) return (false);
	i = 0;	while (A[i]) if (!isupper ((int)A[i++])) return (false);	/* Not all upper case tag */
	i = 0;	while (B[i]) if (!isupper ((int)B[i++])) return (false);	/* Not all upper case tag */
	return (true);	/* Got PLATE_A-PLATE_B specification for GPlates lookup, e.g., IND-CIB */
}

unsigned int spotter_init (struct GMT_CTRL *GMT, char *file, struct EULER **p, bool flowline, bool total_out, bool invert, double *t_max)
{
	/* file;	Name of file with backward stage poles, always GEOCENTRIC */
	/* p;		Pointer to stage pole array */
	/* flowline;	true if flowlines rather than hotspot-tracks are needed */
	/* total_out;	true if we want to return finite (total construction poles) [alternative is stage poles] */
	/* invert;	true if we want to invert all the rotations */
	/* t_max;	Extend earliest stage pole back to this age */
	bool GPlates = false, total_in = false;
	unsigned int n, nf, i = 0, k, id, A_id = 0, B_id = 0, p1, p2, V1 = 0, V2 = 0;
	size_t n_alloc = GMT_SMALL_CHUNK;
	double lon, lat, rot, t, last_t = -DBL_MAX;
	FILE *fp = NULL;
	struct EULER *e = NULL;
	char buffer[GMT_BUFSIZ] = {""}, A[GMT_LEN64] = {""}, B[GMT_LEN64] = {""}, txt[GMT_LEN64] = {""}, comment[GMT_BUFSIZ] = {""};
	char Plates[GMT_BUFSIZ] = {""}, Rotations[GMT_BUFSIZ] = {""}, *this_c = NULL;
	double K[9];

	if (spotter_GPlates_pair (file)) {	/* Got PLATE_A-PLATE_B specification for GPlates lookup, e.g., IND-CIB */
		sscanf (file, "%[^-]-%s", A, B);
		strncpy (Plates, ((this_c = getenv ("GPLATES_PLATES")) != NULL) ? this_c : GPLATES_PLATES, GMT_BUFSIZ);
#ifdef WIN32
		DOS_path_fix (Plates);
#endif
		if ((fp = GMT_fopen (GMT, Plates, "r")) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Cannot open GPlates plate id file %s\n", Plates);
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Using GPlates plate id file %s\n", Plates);
		A_id = B_id = 0;
		while ((A_id == 0 || B_id == 0) && GMT_fgets (GMT, buffer, GMT_BUFSIZ, fp) != NULL) { /* Expects lon lat t0 t1 ccw-angle */
			if (buffer[0] == '#' || buffer[0] == '\n') continue;
			sscanf (buffer, "%d %s %[^\n]", &id, txt, comment);
			if (A_id == 0 && !strcmp (txt, A)) A_id = id;
			if (B_id == 0 && !strcmp (txt, B)) B_id = id;
		}
		GMT_fclose (GMT, fp);
		if (A_id == 0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Could not find an entry for plate %s in the GPlates plate id file\n", A);
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
		if (B_id == 0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Could not find an entry for plate %s in the GPlates plate id file\n", B);
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
		/* OK, here we have the two IDs */
		strncpy (Rotations, ((this_c = getenv ("GPLATES_ROTATIONS")) != NULL) ? this_c : GPLATES_ROTATIONS, GMT_BUFSIZ);
#ifdef WIN32
		DOS_path_fix (Rotations);
#endif
		if ((fp = GMT_fopen (GMT, Rotations, "r")) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Cannot open GPlates rotation file %s\n", Rotations);
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Using GPlates rotation file %s\n", Rotations);
		GPlates = total_in = true;
	}
	else if ((fp = GMT_fopen (GMT, file, "r")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Cannot open stage pole file: %s\n", file);
		GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
	}

	e = GMT_memory (GMT, NULL, n_alloc, struct EULER);
	if (flowline) total_out = true;	/* Override so we get finite poles for conversion to forward stage poles at the end */

	while (GMT_fgets (GMT, buffer, GMT_BUFSIZ, fp) != NULL) { /* Expects lon lat t0 t1 ccw-angle */
		if (buffer[0] == '#' || buffer[0] == '\n') continue;

		if (GPlates) {
			if ((nf = sscanf (buffer, "%d %lf %lf %lf %lf %d %[^\n]", &p1, &t, &lat, &lon, &rot, &p2, comment)) != 7) continue;
			if (GMT_IS_ZERO (t)) continue;	/* Not a rotation */
			if (strstr (comment, "cross-over") || strstr (comment, "cross over") || strstr (comment, "crossover")) continue;	/* Skip GPlates cross-over rotations */
			if (A_id == p1 && B_id == p2 && !V2) {	/* Exactly what we wanted */
				e[i].lon = lon;	e[i].lat = lat;	e[i].omega = rot;	e[i].t_start = t;
				V1 = true;	/* So we dont later find inverse rotations */
			}
			else if (A_id == p2 && B_id == p1 && !V1) {	/* Got the inverse rotation, so change angle sign */
				e[i].lon = lon;	e[i].lat = lat;	e[i].omega = -rot;	e[i].t_start = t;
				V2 = true;
			}
			else
				continue;	/* Not the plate pair we are looking for */
		}
		else {	/* The minimalist record formats is: lon lat t0 [t1] omega [covar] */
			nf = sscanf (buffer, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&e[i].lon, &e[i].lat, &e[i].t_start, &e[i].t_stop, &e[i].omega, &K[0], &K[1], &K[2], &K[3], &K[4], &K[5], &K[6], &K[7], &K[8]);
			if (! (nf == 4 || nf == 5 || nf == 13 || nf == 14)) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Rotation file format must be lon lat t0 [t1] omega [k_hat a b c d e f g df]\n");
				GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
			}
			if (nf == 4 || nf == 13) {	/* total reconstruction format: Got lon lat t0 omega [covars], must shift the K's by one */
				if (i && !total_in) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Rotation file mixes total reconstruction and stage rotation format\n");
					GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
				}
				total_in = true;
				for (k = 8; k > 0; k--) K[k] = K[k-1];
				K[0] = e[i].omega;
				e[i].omega = e[i].t_stop;
				e[i].t_stop = 0.0;
			}
			if (nf > 5) { /* [K = covars] is stored as [k_hat a b c d e f g df] */
				if (K[8] == 0.0) K[8] = 10000.0;	/* No d.f. given */
				record_to_covar (&e[i], K);
			}
			if (total_in && invert) e[i].omega = -e[i].omega;	/* Want the inverse rotation; easy to do if total reconstruction rotations */
		}

		if (total_in && e[i].t_start < last_t) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Rotation %d has time reversal\n", i);
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
		last_t = e[i].t_start;
		if (e[i].t_stop >= e[i].t_start) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Stage rotation %d has start time younger than stop time\n", i);
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
		e[i].duration = e[i].t_start - e[i].t_stop;
		e[i].omega /= e[i].duration;	/* Convert to opening rate */

		e[i].omega_r = e[i].omega * D2R;
		sincosd (e[i].lat, &e[i].sin_lat, &e[i].cos_lat);
		e[i].lon_r = e[i].lon * D2R;
		e[i].lat_r = e[i].lat * D2R;
		if (GPlates) {
			e[i].id[0] = A_id;
			e[i].id[1] = B_id;
		}
		i++;
		if (i == n_alloc) {
			n_alloc <<= 1;
			e = GMT_memory (GMT, e, n_alloc, struct EULER);
		}
	}
	GMT_fclose (GMT, fp);

	if (GPlates && i == 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Could not find rotations for the plate pair %s - %s\n", A, B);
		GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
	}
	
	/* Sort the rotations to make sure they are in the expected order */
	
	n = i;
	if (total_in) {
		qsort (e, n, sizeof (struct EULER), spotter_comp_total);
		invert = false;	/* Since we have taken care of this already */
	}
	else
		qsort (e, n, sizeof (struct EULER), spotter_comp_stage);
	
	if (total_in && !total_out) spotter_total_to_stages (GMT, e, n, true, true);	/* Convert total reconstruction poles to forward stage poles */
	if (!total_in && total_out) {
		spotter_stages_to_total (GMT, e, n, true, true);	/* Convert forward stage poles to total reconstruction poles */
		if (invert) for (i = 0; i < n; i++) {e[i].omega = -e[i].omega; e[i].omega_r = - e[i].omega_r;}
		invert = false;	/* Since we have taken care of this now */
	}
	if (n < n_alloc) e = GMT_memory (GMT, e, n, struct EULER);

	if (invert) {	/* If true this means we read stage rotations and want stage rotations out.  We must take a detour */
		spotter_stages_to_total (GMT, e, n, true, true);	/* Convert forward stage poles to total reconstruction poles */
		for (i = 0; i < n; i++) {e[i].omega = -e[i].omega; e[i].omega_r = - e[i].omega_r;}
		spotter_total_to_stages (GMT, e, n, true, true);	/* Convert total reconstruction poles to forward stage poles */
	}
	
	if (flowline) {	/* Get the forward stage poles from the total reconstruction poles */
		spotter_total_to_fwstages (GMT, e, n, true, true);
	}

	/* Extend oldest stage pole back to t_max Ma */

	if ((*t_max) > 0.0 && e[0].t_start < (*t_max)) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "libspotter: Extending oldest stage pole back to %g Ma\n", (*t_max));

		e[0].t_start = (*t_max);
		e[0].duration = e[0].t_start - e[0].t_stop;
	}
	else
		(*t_max) = MAX (e[0].t_start, e[n-1].t_start);
	*p = e;

	return (n);
}

/* hotspot_init: Reads a file with hotspot information and returns pointer to
 * array of structures.  Hotspot locations are stored as geodetic coordintaes
 * but are converted to GEOCENTRIC by this function if geocentric == true */

unsigned int spotter_hotspot_init (struct GMT_CTRL *GMT, char *file, bool geocentric, struct HOTSPOT **p)
{
	unsigned int i = 0, n;
	int ival;
	size_t n_alloc = GMT_CHUNK;
	FILE *fp = NULL;
	struct HOTSPOT *e = NULL;
	char buffer[GMT_BUFSIZ] = {""}, create, fit, plot;
	double P[3];

	if ((fp = GMT_fopen (GMT, file, "r")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot open file %s - aborts\n", file);
		exit (EXIT_FAILURE);
	}

	e = GMT_memory (GMT, NULL, n_alloc, struct HOTSPOT);

	while (GMT_fgets (GMT, buffer, GMT_BUFSIZ, fp) != NULL) {
		if (buffer[0] == '#' || buffer[0] == '\n') continue;
		n = sscanf (buffer, "%lf %lf %s %d %lf %lf %lf %c %c %c %s", &e[i].lon, &e[i].lat, e[i].abbrev, &ival, &e[i].radius, &e[i].t_off, &e[i].t_on, &create, &fit, &plot, e[i].name);
		if (n == 3) ival = i + 1;	/* Minimal lon, lat, abbrev */
		if (ival <= 0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Hotspot ID numbers must be > 0\n");
			exit (EXIT_FAILURE);
		}
		e[i].id = ival;
		if (n >= 10) {		/* Long-form hotspot table used for rotator suite */
			e[i].create = (create == 'Y');
			e[i].fit = (fit == 'Y');
			e[i].plot = (plot == 'Y');
		}
		if (geocentric) e[i].lat = GMT_lat_swap (GMT, e[i].lat, GMT_LATSWAP_G2O);	/* Convert to geocentric */
		GMT_geo_to_cart (GMT, e[i].lat, e[i].lon, P, true);
		e[i].x = P[0];
		e[i].y = P[1];
		e[i].z = P[2];
		i++;
		if (i == n_alloc) {
			n_alloc <<= 1;
			e = GMT_memory (GMT, e, n_alloc, struct HOTSPOT);
		}
	}
	GMT_fclose (GMT, fp);
	if (i < n_alloc) e = GMT_memory (GMT, e, i, struct HOTSPOT);
	*p = e;

	return (i);
}

int spotter_stage (struct GMT_CTRL *GMT, double t, struct EULER p[], unsigned int ns)
{	/* Return the stage ID for given t, or -1 if not within time range */
	unsigned int j = 0;
	GMT_UNUSED(GMT);
	while (j < ns && t < p[j].t_stop) j++;	/* Find first applicable stage pole */
	if (j == ns) return (-1);	/* Outside in time */
	return (j);
}

/* spotter_backtrack: Given a seamount location and age, trace the
 *	hotspot-track between this seamount and a seamount of 
 *	age t_zero.  For t_zero = 0 this means the hotspot
 */

unsigned int spotter_backtrack (struct GMT_CTRL *GMT, double xp[], double yp[], double tp[], unsigned int np, struct EULER p[], unsigned int ns, double d_km, double t_zero, unsigned int time_flag, double wesn[], double **c)
/* xp, yp;	Points, in RADIANS */
/* tp;		Age of feature in m.y. */
/* np;		# of points */
/* p;		Stage poles */
/* ns;		# of stage poles */
/* d_km;	Create track point every d_km km.  If == -1.0, return bend points only */
/* t_zero;	Backtrack up to this age */
/* time_flag;	1 if we want to interpolate and return time along track, 2 if we just want stage # */
/* wesn:	if time_flag >= 10, only to track within the given box */
/* **c;		Pointer to return track vector */
{
	unsigned int i, stage = 0, k, kk = 0, start_k = 0, nd = 1, nn;
	bool path, bend, go = false, box_check;
	int sideA[2] = {0, 0}, sideB[2] = {0, 0};
	size_t n_alloc = 2 * GMT_CHUNK;
	double t, tt = 0.0, dt, d_lon, tlon, dd = 0.0, i_km = 0.0, xnew, xx, yy, next_x, next_y;
	double s_lat, c_lat, s_lon, c_lon, cc, ss, cs, i_nd, *track = NULL;

	bend = (d_km <= (GMT_CONV4_LIMIT - 1.0));
	path = (bend || d_km > GMT_CONV4_LIMIT);
	if (time_flag >= 10) {	/* Restrict track sampling to given wesn box */
		time_flag -= 10;
		box_check = true;
	}
	else {
		box_check = false;
		go = true;
	}

	if (path) {
		track = GMT_memory (GMT, NULL, n_alloc, double);
		i_km = EQ_RAD / d_km;
	}

	if (p[ns-1].t_stop > t_zero) t_zero = p[ns-1].t_stop;	/* In case we don't go all the way to zero */

	for (i = 0; i < np; i++) {

		if (path) {
			start_k = kk++;
			if (kk == n_alloc) {
				n_alloc <<= 1;
				track = GMT_memory (GMT, track, n_alloc, double);
			}
		}
		nn = 0;

		t = tp[i];

		if (box_check) set_inout_sides (xp[i], yp[i], wesn, sideB);
		while (t > t_zero) {	/* As long as we're not back at zero age */
			if (box_check) sideA[0] = sideB[0], sideA[1] = sideB[1];

			stage = 0;
			while (stage < ns && t <= p[stage].t_stop) stage++;	/* Find first applicable stage pole */
			if (stage == ns) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "(spotter_backtrack) Ran out of stage poles for t = %g\n", t);
				GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
			}
			dt = MIN (p[stage].duration, t - MAX(p[stage].t_stop, t_zero));
			d_lon = p[stage].omega_r * dt;

			xnew = xp[i] - p[stage].lon_r;
			sincos (yp[i], &s_lat, &c_lat);
			sincos (xnew, &s_lon, &c_lon);
			cc = c_lat * c_lon;
			tlon = d_atan2 (c_lat * s_lon, p[stage].sin_lat * cc - p[stage].cos_lat * s_lat);
			s_lat = p[stage].sin_lat * s_lat + p[stage].cos_lat * cc;
			c_lat = sqrt (1.0 - s_lat * s_lat);
			ss = p[stage].sin_lat * s_lat;
			cs = p[stage].cos_lat * s_lat;

			/* Get the next bend point first */

			xnew = tlon + d_lon;
			sincos (xnew, &s_lon, &c_lon);
			cc = c_lat * c_lon;
			next_y = d_asin (ss - p[stage].cos_lat * cc);
			next_x = p[stage].lon_r + d_atan2 (c_lat * s_lon, p[stage].sin_lat * cc + cs);

			if (next_x < 0.0) next_x += TWO_PI;
			if (next_x >= TWO_PI) next_x -= TWO_PI;

			if (box_check) {	/* See if this segment _might_ involve the box in any way; if so do the track sampling */
				set_inout_sides (next_x, next_y, wesn, sideB);
				go = must_do_track (sideA, sideB);
			}
			if (path) {
				if (!bend) {
					nd = urint (ceil ((fabs (d_lon) * c_lat) * i_km));
					i_nd = 1.0 / nd;
					dd = d_lon * i_nd;
					tt = dt * i_nd;
				}
				track[kk++] = xp[i];
				if (kk == n_alloc) {
					n_alloc <<= 1;
					track = GMT_memory (GMT, track, n_alloc, double);
				}
				track[kk++] = yp[i];
				if (kk == n_alloc) {
					n_alloc <<= 1;
					track = GMT_memory (GMT, track, n_alloc, double);
				}
				if (time_flag) {
					track[kk++] = (time_flag == 2) ? (double)(ns - stage) : t;
					if (kk == n_alloc) {
						n_alloc <<= 1;
						track = GMT_memory (GMT, track, n_alloc, double);
					}
				}
				if (!go) nd = 1;
				for (k = 1; go && k < nd; k++) {

					xnew = tlon + k * dd;
					sincos (xnew, &s_lon, &c_lon);
					cc = c_lat * c_lon;
					yy = d_asin (ss - p[stage].cos_lat * cc);
					xx = p[stage].lon_r + d_atan2 (c_lat * s_lon, p[stage].sin_lat * cc + cs);

					if (xx < 0.0) xx += TWO_PI;
					if (xx >= TWO_PI) xx -= TWO_PI;
					track[kk++] = xx;
					if (kk == n_alloc) {
						n_alloc <<= 1;
						track = GMT_memory (GMT, track, n_alloc, double);
					}
					track[kk++] = yy;
					if (kk == n_alloc) {
						n_alloc <<= 1;
						track = GMT_memory (GMT, track, n_alloc, double);
					}
					if (time_flag) {
						track[kk++] = (time_flag == 2) ? (double)(ns - stage) : t - k * tt;
						if (kk == n_alloc) {
							n_alloc <<= 1;
							track = GMT_memory (GMT, track, n_alloc, double);
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
				track = GMT_memory (GMT, track, n_alloc, double);
			}
			track[kk++] = yp[i];
			if (kk == n_alloc) {
				n_alloc <<= 1;
				track = GMT_memory (GMT, track, n_alloc, double);
			}
			if (time_flag) {
				track[kk++] = (time_flag == 2) ? (double)(ns - stage) : t;
				if (kk == n_alloc) {
					n_alloc <<= 1;
					track = GMT_memory (GMT, track, n_alloc, double);
				}
			}
			track[start_k] = (double)(nn+1);
		}
	}
	if (path) {
		track = GMT_memory (GMT, track, kk, double);
		*c = track;
		return (kk);
	}

	return (np);
}

/* spotter_forthtrack: Given a hotspot location and final age, trace the
 *	hotspot-track between the seamount created at t_zero and a
 *	seamount of age tp.  For t_zero = 0 this means from the hotspot.
 */

unsigned int spotter_forthtrack (struct GMT_CTRL *GMT, double xp[], double yp[], double tp[], unsigned int np, struct EULER p[], unsigned int ns, double d_km, double t_zero, unsigned int time_flag, double wesn[], double **c)
/* xp, yp;	Points, in RADIANS */
/* tp;		Age of feature in m.y. */
/* np;		# of points */
/* p;		Stage poles */
/* ns;		# of stage poles */
/* d_km;	Create track point every d_km km.  If == -1.0, return bend points only */
/* t_zero;	Foretrack from this age forward */
/* time_flag;	1 if we want to interpolate and return time along track */
/* wesn:	if time_flag >= 10, only to track within the given box */
/* c;		Pointer to return track vector */
{
	unsigned int i, stage = 0, k, kk = 0, start_k = 0, nd = 1, nn;
	bool path, bend, go = false, box_check;
	int sideA[2] = {0, 0}, sideB[2] = {0, 0};
	size_t n_alloc = BIG_CHUNK;
	double t, tt = 0.0, dt, d_lon, tlon, dd = 0.0, i_km = 0.0, xnew, xx, yy, *track = NULL;
	double s_lat, c_lat, s_lon, c_lon, cc, ss, cs, i_nd, next_x, next_y;

	bend = (d_km <= (GMT_CONV4_LIMIT - 1.0));
	path = (bend || d_km > GMT_CONV4_LIMIT);
	if (time_flag >= 10) {	/* Restrict track sampling to given wesn box */
		time_flag -= 10;
		box_check = true;
	}
	else {
		box_check = false;
		go = true;
	}

	if (path) {
		track = GMT_memory (GMT, NULL, n_alloc, double);
		i_km = EQ_RAD / d_km;
	}

	if (p[ns-1].t_stop > t_zero) t_zero = p[ns-1].t_stop;	/* In case we don't go all the way to zero */

	for (i = 0; i < np; i++) {

		if (path) {
			start_k = kk++;
			if (kk == n_alloc) {
				n_alloc <<= 1;
				track = GMT_memory (GMT, track, n_alloc, double);
			}
		}
		nn = 0;

		t = t_zero;

		if (box_check) set_inout_sides (xp[i], yp[i], wesn, sideB);
		while (t < tp[i]) {	/* As long as we're not back at zero age */
			if (box_check) sideA[0] = sideB[0], sideA[1] = sideB[1];
			stage = ns - 1;
			while (stage && (t + GMT_CONV8_LIMIT) > p[stage].t_start) stage--;
			/* while (stage < ns && (t + GMT_CONV8_LIMIT) < p[stage].t_stop) stage++; */	/* Find first applicable stage pole */
			if (stage == ns) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "(spotter_forthtrack) Ran out of stage poles for t = %g\n", t);
				GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
			}
			dt = MIN (tp[i], p[stage].t_start) - t;	/* Time interval to rotate */
			d_lon = p[stage].omega_r * dt;		/* Rotation angle (radians) */

			xnew = xp[i] - p[stage].lon_r;
			sincos (yp[i], &s_lat, &c_lat);
			sincos (xnew, &s_lon, &c_lon);
			cc = c_lat * c_lon;
			tlon = d_atan2 (c_lat * s_lon, p[stage].sin_lat * cc - p[stage].cos_lat * s_lat);
			s_lat = p[stage].sin_lat * s_lat + p[stage].cos_lat * cc;
			c_lat = sqrt (1.0 - s_lat * s_lat);
			ss = p[stage].sin_lat * s_lat;
			cs = p[stage].cos_lat * s_lat;

			/* Get the next bend point first */

			xnew = tlon - d_lon;
			sincos (xnew, &s_lon, &c_lon);
			cc = c_lat * c_lon;
			next_y = d_asin (ss - p[stage].cos_lat * cc);
			next_x = p[stage].lon_r + d_atan2 (c_lat * s_lon, p[stage].sin_lat * cc + cs);

			if (next_x < 0.0) next_x += TWO_PI;
			if (next_x >= TWO_PI) next_x -= TWO_PI;

			if (box_check) {	/* See if this segment _might_ involve the box in any way; if so do the track sampling */
				set_inout_sides (next_x, next_y, wesn, sideB);
				go = must_do_track (sideA, sideB);
			}
			if (path) {
				if (!bend) {
					nd = urint (ceil ((fabs (d_lon) * c_lat) * i_km));
					i_nd = 1.0 / nd;
					dd = d_lon * i_nd;
					tt = dt * i_nd;
				}
				track[kk++] = xp[i];
				if (kk == n_alloc) {
					n_alloc <<= 1;
					track = GMT_memory (GMT, track, n_alloc, double);
				}
				track[kk++] = yp[i];
				if (kk == n_alloc) {
					n_alloc <<= 1;
					track = GMT_memory (GMT, track, n_alloc, double);
				}
				if (time_flag) {
					track[kk++] = (time_flag == 2) ? (double)(ns - stage) : t;
					if (kk == n_alloc) {
						n_alloc <<= 1;
						track = GMT_memory (GMT, track, n_alloc, double);
					}
				}
				if (!go) nd = 1;
				for (k = 1; go && k < nd; k++) {
					xnew = tlon - k * dd;
					sincos (xnew, &s_lon, &c_lon);
					cc = c_lat * c_lon;
					yy = d_asin (ss - p[stage].cos_lat * cc);
					xx = p[stage].lon_r + d_atan2 (c_lat * s_lon, p[stage].sin_lat * cc + cs);

					if (xx < 0.0) xx += TWO_PI;
					if (xx >= TWO_PI) xx -= TWO_PI;
					track[kk++] = xx;
					if (kk == n_alloc) {
						n_alloc <<= 1;
						track = GMT_memory (GMT, track, n_alloc, double);
					}
					track[kk++] = yy;
					if (kk == n_alloc) {
						n_alloc <<= 1;
						track = GMT_memory (GMT, track, n_alloc, double);
					}
					if (time_flag) {
						track[kk++] = (time_flag == 2) ? (double)(ns - stage) : t + k * tt;
						if (kk == n_alloc) {
							n_alloc <<= 1;
							track = GMT_memory (GMT, track, n_alloc, double);
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
				track = GMT_memory (GMT, track, n_alloc, double);
			}
			track[kk++] = yp[i];
			if (kk == n_alloc) {
				n_alloc <<= 1;
				track = GMT_memory (GMT, track, n_alloc, double);
			}
			if (time_flag) {
				track[kk++] = (time_flag == 2) ? (double)(ns - stage) : t;
				if (kk == n_alloc) {
					n_alloc <<= 1;
					track = GMT_memory (GMT, track, n_alloc, double);
				}
			}
			track[start_k] = (double)(nn+1);
		}
	}
	if (path) {
		track = GMT_memory (GMT, track, kk, double);
		*c = track;
		return (kk);
	}

	return (np);
}

void spotter_total_to_stages (struct GMT_CTRL *GMT, struct EULER p[], unsigned int n, bool finite_rates, bool stage_rates)
{
	/* Convert finite rotations to backwards stage rotations for backtracking */
	/* p[]		: Array of structure elements with rotation parameters
	 * n		: Number of rotations
	 * finite_rates	: true if finite rotations given in degree/my [else we have opening angle]
	 * stage_rates	: true if stage rotations should be returned in degree/my [else we return opening angle]
	 */
	 
	unsigned int i;
	double *elon = NULL, *elat = NULL, *ew = NULL, t_old;
	double R_young[3][3], R_old[3][3], R_stage[3][3];

	/* Expects total reconstruction models to have youngest poles first */

	elon = GMT_memory (GMT, NULL, n, double);
	elat = GMT_memory (GMT, NULL, n, double);
	ew   = GMT_memory (GMT, NULL, n, double);

	set_I_matrix (R_young);		/* The first time, R_young is simply I */

	t_old = 0.0;
	for (i = 0; i < n; i++) {
		if (finite_rates) p[i].omega *= p[i].duration;				/* Convert opening rate to opening angle */
		GMT_make_rot_matrix (GMT, p[i].lon, p[i].lat, p[i].omega, R_old);	/* Get rotation matrix from pole and angle */
		spotter_matrix_mult (GMT, R_young, R_old, R_stage);			/* This is R_stage = R_young^t * R_old */
		spotter_matrix_to_pole (GMT, R_stage, &elon[i], &elat[i], &ew[i]);	/* Get rotation parameters from matrix */
		if (elon[i] > 180.0) elon[i] -= 360.0;					/* Adjust lon */
		spotter_matrix_transpose (GMT, R_young, R_old);				/* Sets R_young = transpose (R_old) for next round */
		p[i].t_stop = t_old;
		t_old = p[i].t_start;
	}

	/* Repopulate the EULER structure given the rotation parameters */

	xyw_to_struct_euler (p, elon, elat, ew, n, true, stage_rates);

	GMT_free (GMT, elon);
	GMT_free (GMT, elat);
	GMT_free (GMT, ew);

	reverse_rotation_order (p, n);	/* Flip order since stages go from oldest to youngest */
}

void spotter_stages_to_total (struct GMT_CTRL *GMT, struct EULER p[], unsigned int n, bool finite_rates, bool stage_rates)
{
	/* Convert stage rotations to finite rotations */
	/* p[]		: Array of structure elements with rotation parameters
	 * n		: Number of rotations
	 * finite_rates	: true if finite rotations should be returned in degree/my [else we return opening angle]
	 * stage_rates	: true if stage rotations given in degree/my [else we have opening angle]
	 */

	unsigned int stage;
	double *elon = NULL, *elat = NULL, *ew = NULL;
	double R_young[3][3], R_old[3][3], R_stage[3][3];

	/* Expects stage pole models to have oldest poles first, so we must flip order */

	reverse_rotation_order (p, n);	/* Expects stage pole models to have oldest poles first, so we must flip order */

	elon = GMT_memory (GMT, NULL, n, double);
	elat = GMT_memory (GMT, NULL, n, double);
	ew   = GMT_memory (GMT, NULL, n, double);

	set_I_matrix (R_old);		/* The first time, R_old is simply I */

	for (stage = 0; stage < n; stage++) {
		if (stage_rates) p[stage].omega *= p[stage].duration;					/* Convert opening rate to opening angle */
		GMT_make_rot_matrix (GMT, p[stage].lon, p[stage].lat, p[stage].omega, R_stage);	/* Make matrix from rotation parameters */
		spotter_matrix_mult (GMT, R_old, R_stage, R_young);					/* Set R_young = R_old * R_stage */
		GMT_memcpy (R_old, R_young, 9, double);							/* Set R_old = R_young for next time around */
		spotter_matrix_to_pole (GMT, R_young, &elon[stage], &elat[stage], &ew[stage]);		/* Get rotation parameters from matrix */
		if (elon[stage] > 180.0) elon[stage] -= 360.0;						/* Adjust lon */
	}

	/* Repopulate the EULER structure given the rotation parameters */

	xyw_to_struct_euler (p, elon, elat, ew, n, false, finite_rates);

	GMT_free (GMT, elon);
	GMT_free (GMT, elat);
	GMT_free (GMT, ew);
}

void spotter_add_rotations (struct GMT_CTRL *GMT, struct EULER a[], int n_a_in, struct EULER b[], int n_b_in, struct EULER *c[], unsigned int *n_c)
{
	/* Takes two finite rotation models and adds them together.
	 * We do this by first converting both to stage poles.  We then
	 * determine all the time knots needed, and then resample both
	 * stage rotation models onto the same list of knots.  This is
	 * easy to do with stage poles since we end up using partial
	 * stage rotations.  To do this with finite poles would involve
	 * computing intermediate stages anyway.  When we have the resampled
	 * stage rotations we convert back to finite rotations and then
	 * simply add each pair of rotations using matrix multiplication.
	 * The final finite rotation model is returned in c. */

	struct EULER *a2 = NULL, *b2 = NULL, *c2 = NULL;
	double *t = NULL, t_min, t_max, Ra[3][3], Rb[3][3], Rab[3][3], lon, lat, w, sign_a, sign_b;
	double tmp[3][3], RaT[3][3], Ca[3][3], Cb[3][3];
	unsigned int i, j, k, n_k = 0, n_a, n_b;
	bool a_ok = true, b_ok = true;

	sign_a = (n_a_in > 0) ? +1.0 : -1.0;
	sign_b = (n_b_in > 0) ? +1.0 : -1.0;
	n_a = abs (n_a_in);
	n_b = abs (n_b_in);
	/* Allocate more than we need, must likely */

	t = GMT_memory (GMT, NULL, n_a + n_b, double);

	/* First convert the two models to stage poles */

	spotter_total_to_stages (GMT, a, n_a, true, true);		/* Return stage poles */
	spotter_total_to_stages (GMT, b, n_b, true, true);		/* Return stage poles */

	/* Find all the time knots used by the two models */

	t_max = MIN (a[0].t_start, b[0].t_start);
	t_min = MAX (a[n_a-1].t_stop, b[n_b-1].t_stop);
	t[n_k++] = t_max;
	i = j = 0;
	while (i < n_a && a[i].t_stop >= t[0]) i++;
	if (i == (n_a - 1)) a_ok = false;
	while (j < n_b && b[j].t_stop >= t[0]) j++;
	if (j == (n_b - 1)) b_ok = false;
	while (a_ok || b_ok) {
		if (a_ok && !b_ok) {		/* Only a left */
			t[n_k] = a[i++].t_stop;
			if (i == (n_a - 1)) a_ok = false;
		}
		else if (b_ok && !a_ok) {	/* Only b left */
			t[n_k] = b[j++].t_stop;
			if (j == (n_b - 1)) b_ok = false;
		}
		else if (a_ok && a[i].t_stop > b[j].t_stop) {
			t[n_k] = a[i++].t_stop;
			if (i == (n_a - 1)) a_ok = false;
		}
		else if (b_ok && b[j].t_stop > a[i].t_stop) {
			t[n_k] = b[j++].t_stop;
			if (j == (n_b - 1)) b_ok = false;
		}
		else {	/* Same time for both */
			t[n_k] = b[j++].t_stop;
			i++;
			if (i == (n_a - 1)) a_ok = false;
			if (j == (n_b - 1)) b_ok = false;
		}
		n_k++;
	}
	t[n_k++] = t_min;
	n_k--;	/* Number of structure elements is one less than number of knots */

	b2 = GMT_memory (GMT, NULL, n_k, struct EULER);
	a2 = GMT_memory (GMT, NULL, n_k, struct EULER);
	c2 = GMT_memory (GMT, NULL, n_k, struct EULER);

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

	GMT_free (GMT, t);

	/* Now switch to finite rotations again to do the additions */

	spotter_stages_to_total (GMT, a2, n_k, false, true);	/* Return opening angles, not rates this time */
	spotter_stages_to_total (GMT, b2, n_k, false, true);

	for (i = 0; i < n_k; i++) {	/* Add each pair of rotations */
		GMT_make_rot_matrix (GMT, a2[i].lon, a2[i].lat, sign_a * a2[i].omega, Ra);
		GMT_make_rot_matrix (GMT, b2[i].lon, b2[i].lat, sign_b * b2[i].omega, Rb);
		spotter_matrix_mult (GMT, Rb, Ra, Rab);	/* Rot a + Rot b = RB * Ra ! */
		spotter_matrix_to_pole (GMT, Rab, &lon, &lat, &w);
		c2[i].lon = lon;
		c2[i].lat = lat;
		c2[i].t_start = a2[i].t_start;
		c2[i].t_stop  = 0.0;
		c2[i].duration = c2[i].t_start;
		c2[i].omega = w / c2[i].duration;	/* Return rates again */
		c2[i].id[0] = a2[i].id[0];
		c2[i].id[1] = b2[i].id[1];
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

			spotter_matrix_transpose (GMT, RaT, Ra);
			if (sign_a < 0.0)
				spotter_cov_of_inverse (GMT, &a2[i], Ca);
			else
				GMT_memcpy (Ca, a2[i].C, 9, double);
			if (sign_b < 0.0)
				spotter_cov_of_inverse (GMT, &b2[i], Cb);
			else
				GMT_memcpy (Cb, b2[i].C, 9, double);
			spotter_matrix_mult (GMT, Cb, Ra, tmp);
			spotter_matrix_mult (GMT, RaT, tmp, c2[i].C);
			for (k = 0; k < 3; k++) for (j = 0; j < 3; j++) c2[i].C[k][j] *= fb;
			for (k = 0; k < 3; k++) for (j = 0; j < 3; j++) tmp[k][j] = fa * Ca[k][j];
			spotter_matrix_add (GMT, c2[i].C, tmp, c2[i].C);
			c2[i].has_cov = true;
			c2[i].g = MIN(a2[i].g, b2[i].g);
		}
	}
	GMT_free (GMT, a2);
	GMT_free (GMT, b2);

	*n_c = n_k;
	*c = c2;
}

double spotter_t2w (struct GMT_CTRL *GMT, struct EULER a[], unsigned int n, double t)
{
	/* Take time, return cumulative omega */

	int i;
	double w = 0.0;
	GMT_UNUSED(GMT);

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

#if 0	/* Seems to be unused */
void set_rot_angle (double w, double R[3][3], double E[])
{	/* Sets R using R(no_omega) and the given rotation angle w in radians */
	double sin_w, cos_w, c, E_x, E_y, E_z;

	sincos (w, &sin_w, &cos_w);
	c = 1.0 - cos_w;

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
#endif

void spotter_matrix_mult (struct GMT_CTRL *GMT, double a[3][3], double b[3][3], double c[3][3])
{	/* C = A * B */
	unsigned int i, j, k;
	GMT_UNUSED(GMT);

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			c[i][j] = 0.0;
			for (k = 0; k < 3; k++) c[i][j] += a[i][k] * b[k][j];
		}
	}
}

void spotter_matrix_transpose (struct GMT_CTRL *GMT, double At[3][3], double A[3][3]) {
	/* Computes the matrix transpose */

	unsigned int i, j;
	GMT_UNUSED(GMT);
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 3; i++) {
			At[i][j] = A[j][i];
		}
	}
}

void spotter_matrix_add (struct GMT_CTRL *GMT, double A[3][3], double B[3][3], double C[3][3]) {
	/* Computes the matrix addition */

	unsigned int i, j;
	GMT_UNUSED(GMT);
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 3; i++) {
			C[i][j] = A[i][j] + B[i][j];
		}
	}
}

void spotter_get_rotation (struct GMT_CTRL *GMT, struct EULER *p, unsigned int np, double t, double *lon, double *lat, double *w)
{	/* Given finite rotations and a time t, return the rotation (lon,lat,w) for that time via interpolation */
	/* We have already checked that t is within range of p */
	unsigned int i;
	struct EULER e[2];
	double R[3][3], dR[3][3], X[3][3], omega;
	
	for (i = 0; i < np && t > p[i].t_start; i++);	/* Wind to first partial rotation stage */
	if (doubleAlmostEqualZero (t, p[i].t_start)) {	/* Hit an exact finite rotation, just return those parameters */
		*lon = p[i].lon;
		*lat = p[i].lat;
		*w = p[i].omega * p[i].duration;
		return;
	}
	if (i == 0) {	/* Just need a partial rotation of the first full rotation */
		*lon = p[0].lon;
		*lat = p[0].lat;
		*w = p[0].omega * t;
		return;
	}
	
	/* Here we must add a partial rotation to the last finite rotation */
	
	i--;
	GMT_memcpy (e, &p[i], 2, struct EULER);	/* Duplicate the two finite rotations bracketing the desired time */
	spotter_total_to_stages (GMT, e, 2, true, true);	/* Convert total reconstruction poles to forward stage poles */
	GMT_make_rot_matrix (GMT, e[1].lon, e[1].lat, e[1].omega * e[1].duration, R);	/* Get matrix R for main rotation */
	omega = e[1].omega * (t - e[0].t_stop);						/* Compute rotation angle for the partial rotation */
	GMT_make_rot_matrix (GMT, e[0].lon, e[0].lat, omega, dR);			/* Get matrix Dr for the partial rotation */
	spotter_matrix_mult (GMT, R, dR, X);						/* Calculate the combined rotation ,X */
	spotter_matrix_to_pole (GMT, X, lon, lat, w);						/* Convert to rotation parameters lon, lat, w */
}

bool spotter_conf_ellipse (struct GMT_CTRL *GMT, double lon, double lat, double t, struct EULER *p, unsigned int np, char flag, bool forward, double out[])
{
	/* Given time and rotation parameters, calculate uncertainty in the
	 * reconstructed point in the form of a confidence ellipse.  To follow
	 * the stuff below, it helps to realize that the covariance matrix C that
	 * is stored with each rotation R is for the rotation R which rotates a
	 * point of age t along a chain back to the hotspot.  However, in this
	 * context (the error in a reconstructed point along the chain) we are
	 * actually using the inverse rotation R^t (negative opening angle).  For
	 * that rotation, the covariance matrix is R * cov(r) * R^t.
	 * forward is true if we rotate from past to now and false if we
	 * rotate from now to the past (e.g., move a hotspot up the chain).
	 */

	unsigned int matrix_dim = 3;
	unsigned int i, j, kk = 3, nrots;
	int k;
	double R[3][3], x[3], y[3], M[3][3], RMt[3][3], Rt[3][3], MRt[3][3], cov[3][3], tmp[3][3], C[9];
	double z_unit_vector[3], EigenValue[3], EigenVector[9], work1[3], work2[3], x_in_plane[3], y_in_plane[3];
	double x_comp, y_comp, w;

	/* Find the unique rotation in question */

	for (i = 0, k = -1; k < 0 && i < np; ++i) if (doubleAlmostEqualZero (p[i].t_start, t))
		k = i;
	if (k == -1) return (true);	/* Did not match finite rotation time */

	/* Make M(x), the skew-symmetric matrix needed to compute cov of rotated point */

	spotter_set_M (GMT, lon, lat, M);

	w = p[k].omega * p[k].duration;
	if (forward) w = -w;	/* Want the inverse rotation */
	GMT_make_rot_matrix (GMT, p[k].lon, p[k].lat, w, R);
	spotter_matrix_transpose (GMT, Rt, R);			/* Get the transpose of R^t */
	if (!forward) {		/* Rotate the point into the present */
		GMT_memcpy (cov, p[k].C, 9, double);	/* The rotation's covarience matrix */
	}
	else {	/* Use inverse rotation to rotate the point from the present to past rotations */
		/* We change the sign of w so then R is actually R^t */
		/* Since we are using the inverse rotation we must first get the cov matrix of the
		   inverse rotation: cov(r^t) = R cov(r) R^t.
		   Here, R actually contains R^t so we need the original R (which we will call R^t) as well. */

		spotter_matrix_mult (GMT, p[k].C, R, tmp);			/* Calculate the cov(r) *R^t product */
		spotter_matrix_mult (GMT, Rt, tmp, cov);			/* cov(r^t) = R^t cov(r) R */
	}

	/* Calculate cov(y) = R * M^T * cov_R * M * R^T */

	spotter_matrix_mult (GMT, M, Rt, MRt);		/* Calculate the M * R^T product */
	spotter_matrix_transpose (GMT, RMt, MRt);		/* Get the transpose (M*R^T)^T = R * M^T */
	spotter_matrix_mult (GMT, cov, MRt, tmp);		/* Get C * M * R^T */
	spotter_matrix_mult (GMT, RMt, tmp, M);		/* Finally get R * M * C * M^T * R^T, store result in M */

	for (i = 0; i < 3; i++) for (j = 0; j < 3; j++) C[3*i+j] = M[i][j];	/* Reformat to 1-D format for GMT_jacobi */

	/* Get projected point y = R*x */

	GMT_geo_to_cart (GMT, lat, lon, x, true);
	for (i = 0; i < 3; i++) y[i] = R[i][0] * x[0] + R[i][1] * x[1] + R[i][2] * x[2];
        GMT_cart_to_geo (GMT, &out[1], &out[0], y, true);
	if (flag == 't')
		out[2] = t;
	else if (flag == 'a')
		out[2] = w;
	else
		kk = 2;

	GMT_jacobi (GMT, C, matrix_dim, matrix_dim, EigenValue, EigenVector, work1, work2, &nrots);	/* Solve eigen-system C = EigenVector * EigenValue * EigenVector^T */

	z_unit_vector[0] = z_unit_vector[1] = 0.0;	z_unit_vector[2] = 1.0;	/* z unit vector */
	GMT_cross3v (GMT, z_unit_vector, y, x_in_plane);	/* Local x-axis in plane normal to mean pole */
	GMT_cross3v (GMT, y, x_in_plane, y_in_plane);	/* Local y-axis in plane normal to mean pole */
	x_comp = GMT_dot3v (GMT, EigenVector, x_in_plane);	/* x-component of major axis in tangent plane */
	y_comp = GMT_dot3v (GMT, EigenVector, y_in_plane);	/* y-component of major axis in tangent plane */
	out[kk] = fmod (360.0 + (90.0 - atan2d (y_comp, x_comp)), 360.0);	/* Azimuth of major axis */
	if (out[kk] > 180.0) out[kk] -= 180.0;
	out[++kk] = 2.0 * sqrt (EigenValue[0]) * EQ_RAD * SQRT_CHI2;	/* Report full-length major axis (not semi) */
	out[++kk] = 2.0 * sqrt (EigenValue[1]) * EQ_RAD * SQRT_CHI2;	/* Report full-length minor axis (not semi) */

	return (false);
}

void spotter_matrix_1Dto2D (struct GMT_CTRL *GMT, double *M, double X[3][3]) {
	unsigned int i, j;
	GMT_UNUSED(GMT);
	for (i = 0; i < 3; i++) for (j = 0; j < 3; j++) X[i][j] = M[3*i+j];
}

void spotter_matrix_2Dto1D (struct GMT_CTRL *GMT, double *M, double X[3][3]) {
	unsigned int i, j;
	GMT_UNUSED(GMT);
	for (i = 0; i < 3; i++) for (j = 0; j < 3; j++) M[3*i+j] = X[i][j];
}

void spotter_inv_cov (struct GMT_CTRL *GMT, double Ci[3][3], double C[3][3])
{	/* Return the inverse of a covariance matrix
	 * (or any symmetric 3x3 matrix) */
	
	double inv_D;
	GMT_UNUSED(GMT);
	inv_D = 1.0 / (-C[0][0]*C[1][1]*C[2][2] + C[0][0]*C[1][2]*C[1][2] + C[0][1]*C[0][1]*C[2][2] - 2.0*C[0][1]*C[0][2]*C[1][2] + C[0][2]*C[0][2]*C[1][1]);
	Ci[0][0] = (-C[1][1] * C[2][2] + C[1][2]*C[1][2]) * inv_D;
	Ci[0][1] = Ci[1][0] = (C[0][1]*C[2][2] - C[0][2]*C[1][2]) * inv_D;
	Ci[0][2] = Ci[2][0] = (C[0][2]*C[1][1] - C[0][1]*C[1][2]) * inv_D;
	Ci[1][1] = (C[0][2]*C[0][2] - C[0][0]*C[2][2]) * inv_D;
	Ci[1][2] = Ci[2][1] = (C[0][0]*C[1][2] - C[0][1]*C[0][2]) * inv_D;
	Ci[2][2] = (C[0][1]*C[0][1] - C[0][0]*C[1][1]) * inv_D;	
}

unsigned int spotter_confregion_radial (struct GMT_CTRL *GMT, double alpha, struct EULER *p, double **X, double **Y)
{	/* RADIAL PROJECTION */
	/* alpha:	Level of significance, e.g., 0.95 for 95% confidence region */
	/* p:		Euler rotation structure for the current rotation */
	/* X, Y:	Pointers to arrays that will hold the confidence region polygon */
		
	unsigned int i, j, ii, jj, na, try, n, matrix_dim = 3, nrots, dump = 0, fake = 0, axis[3];
	bool done, got_it, bail;
	size_t n_alloc;
	char *name = "uvw";
	double sa, ca, angle, d, V[3][3], Vt[3][3], C[9], fval = 0.0005;
	double EigenValue[3], EigenVector[9], work1[3], work2[3], r_center[3];
	double axis_length[3], i_axis_length[3], r_t[3], delta, b1, b2, num, radix, c2, t1, t2, uvw[3], N[3], N_xyz[3];
	double uvw_prime[3], r_tangent_path[3], s, c, omega, new_angle, r_center_unit[3], r_t_unit[3];
	double phi[SPOTTER_N_FINE_STEPS+1], t[SPOTTER_N_FINE_STEPS+1], t_inner[SPOTTER_N_FINE_STEPS+1], *lon = NULL, *lat = NULL;
	FILE *fp = NULL;

	double nu = 3.0;	/* Three degrees of freedom for the rotation pole */

#if 0
	if (rot_debug) spotter_confregion_radial_debug (alpha, p);	/* For testing purposes only */
#endif	
	c2 = GMT_chi2crit (GMT, alpha, nu);
	c = sqrt (c2);

	spotter_matrix_2Dto1D (GMT, C, p->C);			/* Reformat p->C to 1-D format C for GMT_jacobi */
	if (fake) {
		GMT_memset (C, 9, double);
		C[0] = C[4] = C[8] = fval;
	}
	GMT_jacobi (GMT, C, matrix_dim, matrix_dim, EigenValue, EigenVector, work1, work2, &nrots);	/* Solve eigen-system C = EigenVector * EigenValue * EigenVector^T */
	spotter_matrix_1Dto2D (GMT, EigenVector, Vt);	/* Reformat back to 2-D format */
	spotter_matrix_transpose (GMT, V, Vt);		/* Get the transpose of V */
	GMT_geo_to_cart (GMT, p->lat_r, p->lon_r, r_center_unit, false);	/* Rotation pseudo-vector */
	omega = p->omega * p->duration;
	for (ii = 0; ii < 3; ii++) {
		r_center[ii] = r_center_unit[ii] * D2R * omega;
		axis_length[ii] = sqrt (EigenValue[ii]);
		i_axis_length[ii] = 1.0 / axis_length[ii];
	}
	GMT_matrix_vect_mult (GMT, 3U, Vt, r_center, r_t);		/* r_center expressed in eigen coordinates */
	GMT_matrix_vect_mult (GMT, 3U, Vt, r_center_unit, r_t_unit);	/* unit vector r_center expressed in eigen coordinates */

	/* Determine which of u, v, and w unit vectors are most parallel with r_t, then use the two other axes for the
	 * loop over angles. The two horizontal axes are indicated by the indices axis[]GMT_X] and axis[GMT_Y], with
	 * axis[GMT_Z] being the axis most parallel with r_t .*/

	GMT_memset (N, 3, double);	N[0] = 1.0;
	angle = GMT_dot3v (GMT, r_t_unit, N);	axis[GMT_Z] = GMT_X;
	GMT_memset (N, 3, double);	N[1] = 1.0;
	new_angle = GMT_dot3v (GMT, r_t_unit, N);
	if (new_angle > angle) {axis[GMT_Z] = GMT_Y; angle = new_angle;}
	GMT_memset (N, 3, double);	N[2] = 1.0;
	new_angle = GMT_dot3v (GMT, r_t_unit, N);
	if (new_angle > angle) {axis[GMT_Z] = GMT_Z; angle = new_angle;}
	axis[GMT_X] = (axis[GMT_Z] == 0) ? 1 : 0;	/* u will be either first or second original axis */
	axis[GMT_Y] = axis[GMT_X] + 1;				/* Let v be the next (either second or third) */
	if (axis[GMT_Y] == axis[GMT_Z]) axis[GMT_Y]++;	/* Occupied by w, go to next */
	GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Spinning in the %c-%c coordinate system\n", name[axis[GMT_X]], name[axis[GMT_Y]]);
	
	/* We will loop over 360 degrees and determine where the radial vector intersects the projected ellipse P'(u,v)
	 * (1) If the origin (0,0) is inside P' then we will always get two real roots that differ in sign. in that case
	 * we simply always go with the positive root for all angles.  (2) If origin is outside P'(u,v) then there will
	 * be angles for which the radius vector does not intersect the curve (and we get two complex roots to ignore),
	 * otherwise the real roots come in pairs of the same sign.  Since the negative pairs repeat the information
	 * of hte positive pairs (except they are 180 degrees shifted) we only keep the positive pairs.  These two
	 * roots represents two different angles 180 degrees apart so therefore we store both and do the stitching
	 * further down.
	*/
	
	na = SPOTTER_N_STEPS;	/* Initial trial */
	done = bail = false;
	try = 0;
	do {
		delta = TWO_PI / na;
		try++;
		for (i = j = 0; i <= na; i++) {
			sincos ((angle = i * delta), &sa, &ca);
			b1 = r_t[axis[GMT_X]] * ca * i_axis_length[axis[GMT_X]] + r_t[axis[GMT_Y]] * sa * i_axis_length[axis[GMT_Y]];
			b2 = r_t[axis[GMT_Z]] * i_axis_length[axis[GMT_Z]];
			num = b1 * b1 + b2 * b2;
			radix = num - c2;
			if (radix < 0.0) continue;	/* Complex roots not interesting here */
			s = b2 * c * sqrt (radix);
			b1 *= c2;
			t1 = (-b1 + s) / num;
			t2 = (-b1 - s) / num;
			/* OK, determine a valid series of t(theta) */
			if ((t1*t2) < 0.0) {    /* Easy, just do the root with positive t */
				t[j] = MAX (t1,t2);		/* Only pick the positive solution */
  				phi[j] = angle;
				j++;
				done = true;	/* No need to redo at finer spacing */
			}
			else if (na == SPOTTER_N_STEPS) {	/* Only gets here in case 2 */
				/* We get here immediately and j = 0.  Break out and redo with more points to handle grazing angles */
				na = SPOTTER_N_FINE_STEPS;
				break;
			}
			else if (t1 >= 0.0) {	/* Two positive roots, we have inner/outer branch */
				t[j] = MAX (t1,t2);		/* Pick the largest for outer branch */
  				phi[j] = angle;
				t_inner[j] = MIN(t1,t2);	/* Pick the smallest for inner branch */
				j++;
				done = true;	/* Since we now are using the finer spacing */
			}
		}
		if (j == 0 && try == 2) {	/* Found no roots at all - bail */
			done = bail = true;
		}
	} while (!done);
	if (bail) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Could not obtain confidence regions for rotation at t = %.2f\n", p->t_start);
		return (0);
	}
	if (na == SPOTTER_N_FINE_STEPS) {	/* Two branches, stitch together */
		i = j;
		while (j > 0) {	/* Wind backwards to reverse t_inner and append to t */
			j--;
			t[i] = t_inner[j];
			phi[i] = phi[j];
			i++;
		}
		na = i;
	}
	/* Here we have a valid t(phi) relationship.  Build output arrays lon,lat */
	
	n_alloc = na;
	lon = GMT_memory (GMT, NULL, n_alloc, double);
	lat = GMT_memory (GMT, NULL, n_alloc, double);
	n = 0;
	if (dump) fp = fopen ("dump_r.txt","w");
	for (i = 0; i < na; i++) {
		sincos (phi[i], &sa, &ca);
		uvw[axis[GMT_X]] = axis_length[axis[GMT_X]] * ca * t[i];
		uvw[axis[GMT_Y]] = axis_length[axis[GMT_Y]] * sa * t[i];
		uvw[axis[GMT_Z]] = axis_length[axis[GMT_Z]] * sqrt (c2 - t[i]*t[i]);
		if (dump) fprintf (fp, "%g\t%g\t%g\n", uvw[0], uvw[1], uvw[2]);
		got_it = false;
		try = 0;
		while (!got_it && try < 2) {
			try++;
			uvw[axis[GMT_Z]] = -uvw[axis[GMT_Z]];	/* Try the opposite sign of last time */
			spotter_ellipsoid_normal (GMT, uvw, axis_length, c, N);	/* Get normal vector */
			GMT_matrix_vect_mult (GMT, 3U, V, N, N_xyz);		/* Upper normal in (x,y,z) coordinates */
			GMT_matrix_vect_mult (GMT, 3U, V, uvw, uvw_prime);	/* potential tangent point in (x,y,z) coordinates */
			for (jj = 0; jj < 3; jj++) r_tangent_path[jj] = uvw_prime[jj] + r_center[jj];	/* r to (upper) tangent path in (x,y,z) coordinates */
			d = fabs (GMT_dot3v (GMT, N_xyz, r_tangent_path));
			got_it = (d < SPOTTER_D_CUT);	/* The surface normal at (u,v,+w) is normal to the position vector */
		}
		if (got_it) {	/* Got a valid point, now compute geographical coordinates */
			GMT_normalize3v (GMT, r_tangent_path);
			GMT_cart_to_geo (GMT, &lat[n], &lon[n], r_tangent_path, true);
			n++;
			if (n == n_alloc) {
				n_alloc <<= 1;
				lon = GMT_memory (GMT, lon, n_alloc, double);
				lat = GMT_memory (GMT, lat, n_alloc, double);
			}
		}
		else {
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "No (u,v,w) solution for angle %g\n", phi[i]);
		}
	}
	if (dump) fclose (fp);
	if (GMT_polygon_is_open (GMT, lon, lat, n)) {	/* Must close the polygon */
		lon[n] = lon[0];
		lat[n] = lat[1];
		n++;
	}
	if (n < n_alloc) {
		lon = GMT_memory (GMT, lon, n, double);
		lat = GMT_memory (GMT, lat, n, double);
	}
	
	*X = lon;
	*Y = lat;
	return (n);
}			

unsigned int spotter_confregion_ortho (struct GMT_CTRL *GMT, double alpha, struct EULER *p, double **X, double **Y)
{	/* ORTHOGRAPHIC PROJECTION */
	/* alpha:	Level of significance, e.g., 0.95 for 95% confidence region */
	/* p:		Euler rotation structure for the current rotation */
	/* X, Y:	Pointers to arrays that will hold the confidence region polygon */
		
	unsigned int i;
#ifdef DEBUG
	bool dump = true;
#endif
	double sa, ca, angle = 0.0, R[3][3], Rt[3][3], T[3][3], C[3][3];
	double par[3], delta, mu, dx_local, dy_local, dr_local, azimuth, dr_dist, sin_phi, cos_phi;
	double *lon = NULL, *lat = NULL;
#ifdef DEBUG
	FILE *fp = NULL;
#endif

	double nu = 3.0;	/* Three degrees of freedom for the rotation pole */
	
	mu = sqrt (GMT_chi2crit (GMT, alpha, nu));	/* Scaling of all axes */
	spotter_tangentplane (GMT, p->lon, p->lat, R);	/* Rotation that relates tangent plane coordinates (p,q,s) with Earth (x,y,z) */
	spotter_matrix_transpose (GMT, Rt, R);				/* Get the transpose of R */
	spotter_matrix_mult (GMT, R, p->C, T);				/* Compute C' = R * C * Rt */
	spotter_matrix_mult (GMT, T, Rt, C);
	spotter_project_ellipsoid_new (GMT, C, par);	/* Get projection of ellipsoid onto tangent plane */
	sincosd (par[0], &sin_phi, &cos_phi);
	
#ifdef DEBUG
	if (dump) fp = fopen ("dump_o.txt","w");
#endif
	delta = 360.0 / (SPOTTER_N_STEPS-1);
	lon = GMT_memory (GMT, NULL, SPOTTER_N_STEPS, double);
	lat = GMT_memory (GMT, NULL, SPOTTER_N_STEPS, double);
	mu /= (p->duration * p->omega * D2R);	/* Scale up so vector touches the Earth's surface (r = 1) */
	for (i = 0; i < SPOTTER_N_STEPS; i++) {
		sincosd ((angle = i * delta), &sa, &ca);
		dx_local = mu * (par[1] * ca * cos_phi - par[2] * sa * sin_phi);
		dy_local = mu * (par[2] * sa * cos_phi + par[1] * ca * sin_phi);
		dr_local  = hypot (dx_local, dy_local);
		dr_dist = R2D * d_asin (dr_local);	/* Undo orthographic projection to get great-circle distance */
		azimuth = atan2 (dy_local, dx_local) * R2D;
		/* Determine lon,lat of a point dr_dist away in the azim direction from center lon,lat */
		GMT_get_point_from_r_az (GMT, p->lon, p->lat, dr_dist, azimuth, &lon[i], &lat[i]);
#ifdef DEBUG
		if (dump) fprintf (fp, "%g\t%g\t%g\t%g\t%g\t%g\n", dx_local, dy_local, dr_dist, azimuth, lon[i], lat[i]);
#endif
	}
#ifdef DEBUG
	if (dump) fclose (fp);
#endif
		
	*X = lon;
	*Y = lat;
	return (i);
}			

void spotter_project_ellipsoid (struct GMT_CTRL *GMT, double axis[], double D[3][3], double *par)
{	/* Project an arbitrarily oriented ellipsoid orthographically onto a plane
	 * using the method of Gendzwill and Stauffer, 1981, "Analysis of triaxial
	 * ellipsoids: Their shapes, plane sections, and plane projections", Math.
	 * Geol., 13 (2), 135-152.
	 * axes: The three axes lengths (eigenvalues)
	 * D: The rotation matrix that relates the (J, K, L) coordinates of the
	 * ellipsoid to the E, N, V coordinates of the real world.
	 * par: Returns azimuth, major, minor axes in the E-N plane.
	 * Note: I rederived to project onto E-N rather than E-V.
	 */
	bool override = false;
	double A, B, C, F, G, H, a2, b2, c2, r, tmp[3][3];
	
	if (override) {
		spotter_matrix_transpose (GMT, tmp, D);
		GMT_memcpy (D, tmp, 9, double);
	}
	
	/* Get square of each axis */
	
	a2 = axis[0] * axis[0];	b2 = axis[1] * axis[1];	c2 = axis[2] * axis[2];
	
	/* Get F, G, H, per equation (20) */
	
	F = D[0][0] * D[0][2] / a2 + D[1][0] * D[1][2] / b2 + D[2][0] * D[2][2] / c2;
	G = D[0][1] * D[0][2] / a2 + D[1][1] * D[1][2] / b2 + D[2][1] * D[2][2] / c2;
	H = D[0][2] * D[0][2] / a2 + D[1][2] * D[1][2] / b2 + D[2][2] * D[2][2] / c2;
	
	/* Then get A, B, C per equation (23) */
	
	A = pow (D[0][0] - D[0][2] * F / H, 2.0) / a2 + pow (D[1][0] - D[1][2] * F / H, 2.0) / b2 + pow (D[2][0] - D[2][2] * F / H, 2.0) / c2;
	B = 2.0 * (D[0][0] - D[0][2] * F / H) * (D[0][1] - D[0][2] * G / H) / a2 +
	    2.0 * (D[1][0] - D[1][2] * F / H) * (D[1][1] - D[1][2] * G / H) / b2 +
		2.0 * (D[2][0] - D[2][2] * F / H) * (D[2][1] - D[2][2] * G / H) / c2;
	C = pow (D[0][1] - D[0][2] * G / H, 2.0) / a2 + pow (D[1][1] - D[1][2] * G / H, 2.0) / b2 + pow (D[2][1] - D[2][2] * G / H, 2.0) / c2;
	
	r = sqrt (A*A - 2*A*C + C*C + 4*B*B);
	par[1] = 1.0/sqrt (0.5 * (A + C + r));
	par[2] = 1.0/sqrt (0.5 * (A + C - r));
	par[0] = (GMT_IS_ZERO (B)) ? ((A > C) ? 90.0 : 0.0) : 90.0 - atan2 (-0.5 * (A - C - r)/B, 1.0) * R2D;
	if (par[2] > par[1]) {	/* Switch so that par[1] is the major axis */
		double_swap (par[1], par[2]);
		par[0] += 90.0;
		if (par[0] >= 180.0) par[0] -= 180.0;
	}
}

void spotter_project_ellipsoid_new (struct GMT_CTRL *GMT, double X[3][3], double *par)
{	/* Project an arbitrarily oriented ellipsoid orthographically onto the x-y plane
	 */
	double a, b, c, r;
	GMT_UNUSED(GMT);

	a = X[0][0] - (X[0][2] * X[0][2] / X[2][2]);
	b = X[0][1] - (X[0][2] * X[1][2] / X[2][2]);
	c = X[1][1] - (X[1][2] * X[1][2] / X[2][2]);
	r = sqrt (a*a - 2.0*a*c + c*c + 4.0*b*b);
	par[1] = sqrt (0.5 * (a + c + r));
	par[2] = sqrt (0.5 * (a + c - r));
	par[0] = (GMT_IS_ZERO (b)) ? ((a > c) ? 90.0 : 0.0) : 90.0 - atan2 (-0.5 * (a - c - r)/b, 1.0) * R2D;
	if (par[2] > par[1]) {	/* Switch so that par[1] is the major axis */
		double_swap (par[1], par[2]);
		par[0] += 90.0;
		if (par[0] >= 180.0) par[0] -= 180.0;
	}
}

void spotter_tangentplane (struct GMT_CTRL *GMT, double lon, double lat, double R[3][3])
{
	/* Returns the rotation R that will relate the Earth X = (x,y,z)
	 * coordinates to the local tangent plane cooardinates T = (tx,ty,tz)
	 * at the point (lon,lat).  Here tx is horizontal (east) coordinate
	 * ty is vertical (north) and tz is radially up.  Usage:
	 *    T = R * X
	 */

	double sa, ca, Rlat[3][3], Rlon[3][3];
		
	sincosd (lat, &sa, &ca);
	Rlat[0][0] = 1.0;	Rlat[0][1] = 0.0;	Rlat[0][2] = 0.0;
	Rlat[1][0] = 0.0;	Rlat[1][1] = -sa;	Rlat[1][2] = ca;
	Rlat[2][0] = 0.0;	Rlat[2][1] = ca;	Rlat[2][2] = sa;
	sincosd (lon, &sa, &ca);
	Rlon[0][0] = -sa;	Rlon[0][1] = ca;	Rlon[0][2] = 0.0;
	Rlon[1][0] = ca;	Rlon[1][1] = sa;	Rlon[1][2] = 0.0;
	Rlon[2][0] = 0.0;	Rlon[2][1] = 0.0;	Rlon[2][2] = 1.0;
	spotter_matrix_mult (GMT, Rlat, Rlon, R);			/* R converts between (x,y,z) and( tx, ty, tz) coordinates in tangent plane */
}

bool on_the_ellipse (double xyz[3], double L[3], double c)
{
	unsigned int i;
	double sum;
	sum = c * c;
	for (i = 0; i < 3; i++) sum -= pow (xyz[i]/L[i],2.0);
	return (GMT_IS_ZERO (sum));
}

void spotter_ellipsoid_normal (struct GMT_CTRL *GMT, double X[3], double L[3], double c, double N[3])
{	/* Compute normal vector N at given point X ON the ellipse */
	if (!on_the_ellipse (X, L, c)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Point X is not on the ellipsoid in ellipsoid_normal!");
		return;
	}
	if (GMT_IS_ZERO (X[GMT_Z])) {	/* Normal vector lies entirely in (x-y) plane */
		if (GMT_IS_ZERO (X[GMT_Y])) {	/* Special case where N is aligned with x-axis */
			N[GMT_X] = copysign (1.0, X[GMT_X]);	/* Pointing in the sign(x) direction */
			N[GMT_Y] = N[GMT_Z] = 0.0;
		}
		else {	/* May compute dy/dx to get tx = (1, dy/dx) so n = (dydx,1,0) with sign taken from quadrants */
			N[GMT_X] = copysign (fabs (L[GMT_Y]*L[GMT_Y]*X[GMT_X]/(L[GMT_X]*L[GMT_X]*X[GMT_Y])), X[GMT_X]);
			N[GMT_Y] = copysign (1.0, X[GMT_Y]);
			N[GMT_Z] = 0.0;
		}
	}
	else {	/* Straight forward solution */
		double L2, tx[3], ty[3];
		tx[GMT_X] = 1.0;
		tx[GMT_Y] = 0.0;
		L2 = L[GMT_Z]*L[GMT_Z];
		tx[GMT_Z] = -L2*X[GMT_X]/(L[GMT_X]*L[GMT_X]*X[GMT_Z]);
		ty[GMT_X] = 0.0;
		ty[GMT_Y] = 1.0;
		ty[GMT_Z] = -L2*X[GMT_Y]/(L[GMT_Y]*L[GMT_Y]*X[GMT_Z]);
		GMT_cross3v (GMT, tx, ty, N);	/* Normal is cross-product of x and y tangent vectors */
	}
}
