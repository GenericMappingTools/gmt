/*
 *	$Id: polygon_xover.c,v 1.4 2004-09-06 23:19:32 pwessel Exp $
 */
/* polygon_xover checks for propoer closure and crossings
 * within polygons
 *
 */

#include "wvs.h"

struct POLYGON {
	struct GMT3_POLY h;
	double *lon;
	double *lat;
} P[N_POLY];

int nothing_in_common (struct GMT3_POLY *h1, struct GMT3_POLY *h2, double *shift);

main (int argc, char **argv)
{
	FILE	*fp;
	int	i, n_id, id1, id2, nx, nx_tot, ANTARCTICA, verbose, full;
	int np_o, np_i1, np_i2, in;
	double x_shift = 0.0, lon_o[N_EUR_O], lat_o[N_EUR_O], lon_i1[N_EUR_I], lat_i1[N_EUR_I];
	double lon_i2[N_EUR_I], lat_i2[N_EUR_I];
	struct GMT_XSEGMENT *ylist1, *ylist2;
	struct GMT_XOVER XC;
	struct LONGPAIR p;

	if (argc < 2 || argc > 3) {
		fprintf(stderr,"usage:  polygon_xover wvs_polygons.b [-V] > report.lis\n");
		exit(-1);
	}

	/* Open file and read everything into memory */
	
	fp = fopen(argv[1], "r");
	verbose = (argc == 3);
	n_id = 0;
	while (pol_readheader (&P[n_id].h, fp) == 1) {
		P[n_id].lon = (double *) GMT_memory (VNULL, P[n_id].h.n, sizeof (double), "polygon_xover");
		P[n_id].lat = (double *) GMT_memory (VNULL, P[n_id].h.n, sizeof (double), "polygon_xover");
		if (fabs (P[n_id].h.east - P[n_id].h.west) == 360.0) ANTARCTICA = n_id;

		for (i = 0; i < P[n_id].h.n; i++) {
			if (pol_fread (&p, 1, fp) != 1) {
				fprintf(stderr,"polygon_xover:  ERROR  reading file.\n");
				exit(-1);
			}
			if (P[n_id].h.greenwich && p.x > P[n_id].h.datelon) p.x -= M360;
			P[n_id].lon[i] = p.x * 1e-6;
			P[n_id].lat[i] = p.y * 1e-6;
		}
		n_id++;
	}
	fclose(fp);

	if (verbose) fprintf (stderr, "polygon_xover: Found %d polygons\n", n_id);
	
	/* Now do double do-loop to find all xovers */
	
	full = (P[0].h.n > 1000000);	/* Only the full resolution has more than 1 mill points for EUR-AFR polygon */
	
	nx_tot = 0;
	for (id1 = 0; id1 < n_id; id1++) {
		if (id1 == ANTARCTICA) continue;	/* Skip Antarctica */
		ylist1 = GMT_init_track (P[id1].lon, P[id1].lat, P[id1].h.n);
		if (full && id1 == 0) {	/* Eurafrica */
			for (i = 0; i < N_EUR_O; i++) lon_o[i] = ieur_o[0][i] - 360.0;
			for (i = 0; i < N_EUR_O; i++) lat_o[i] = ieur_o[1][i];
			np_o = N_EUR_O;
			for (i = 0; i < N_EUR_I; i++) lon_i1[i] = ieur_i[0][i] - 360.0;
			for (i = 0; i < N_EUR_I; i++) lat_i1[i] = ieur_i[1][i];
			np_i1 = N_EUR_I;
			for (i = 0; i < N_AFR_I; i++) lon_i1[i] = iafr_i[0][i] - 360.0;
			for (i = 0; i < N_AFR_I; i++) lat_i1[i] = iafr_i[1][i];
			np_i2 = N_AFR_I;
		}
		else if (full && id1 == 1) {	/* Americas */
			for (i = 0; i < N_AM_O; i++) lon_o[i] = iam_o[0][i];
			for (i = 0; i < N_AM_O; i++) lat_o[i] = iam_o[1][i];
			np_o = N_AM_O;
			for (i = 0; i < N_NAM_I; i++) lon_i1[i] = inam_i[0][i];
			for (i = 0; i < N_NAM_I; i++) lat_i1[i] = inam_i[1][i];
			np_i1 = N_NAM_I;
			for (i = 0; i < N_SAM_I; i++) lon_i1[i] = isam_i[0][i];
			for (i = 0; i < N_SAM_I; i++) lat_i1[i] = isam_i[1][i];
			np_i2 = N_SAM_I;
		}
		else if (full && id1 == 3) {	/* Australia */
			for (i = 0; i < N_AUS_O; i++) lon_o[i] = iaus_o[0][i];
			for (i = 0; i < N_AUS_O; i++) lat_o[i] = iaus_o[1][i];
			np_o = N_AUS_O;
			for (i = 0; i < N_AUS_I; i++) lon_i1[i] = iaus_i[0][i];
			for (i = 0; i < N_AUS_I; i++) lat_i1[i] = iaus_i[1][i];
			np_i1 = N_AUS_I;
			np_i2 = 0;
		}
		else
			np_o = np_i1 = np_i2 = 0;
			
		for (id2 = MAX (4, id1 + 1); id2 < n_id; id2++) {	/* Dont start earlier than 4 since no point comparing Eur to Americas */
			if (id2 == ANTARCTICA) continue;	/* Skip Antarctica */
		
			if (nothing_in_common (&P[id1].h, &P[id2].h, &x_shift)) continue;	/* No area in common */

			/* GMT_non_zero_winding returns 2 if inside, 1 if on line, and 0 if outside */
			
			if (np_o) {	/* Check if outside the current crude polgon first */
				for (i = in = 0; i < P[id2].h.n; i++) in += GMT_non_zero_winding (P[id2].lon[i] + x_shift, P[id2].lat[i], lon_o, lat_o, np_o);
				if (in == 0) continue;	/* Polygon id2 completely outside the "outside" polygon */
			}
			if (np_i1) {	/* Check if inside the first of possibly two crude polgons */
				for (i = in = 0; i < P[id2].h.n; i++) in += GMT_non_zero_winding (P[id2].lon[i] + x_shift, P[id2].lat[i], lon_i1, lat_i1, np_i1);
				if (in == (2 * P[id2].h.n)) continue;	/* Polygon id2 completely inside the "inside" polygon 1 */
			}
			if (np_i2) {	/* Check if inside the 2nd crude polgon */
				for (i = in = 0; i < P[id2].h.n; i++) in += GMT_non_zero_winding (P[id2].lon[i] + x_shift, P[id2].lat[i], lon_i2, lat_i2, np_i2);
				if (in == (2 * P[id2].h.n)) continue;	/* Polygon id2 completely inside the "inside" polygon 1 */
			}
			
			/* Get here when no cheap determination worked and we must do full crossover calculation */
			
			if (verbose) fprintf (stderr, "polygon_xover: %6d vs %6d [T = %6d]\r", P[id1].h.id, P[id2].h.id, nx_tot);
			if (fabs (x_shift) > GMT_CONV_LIMIT) for (i = 0; i < P[id2].h.n; i++) P[id2].lon[i] += x_shift;
			
			ylist2 = GMT_init_track (P[id2].lon, P[id2].lat, P[id2].h.n);

			nx = GMT_crossover (P[id1].lon, P[id1].lat, NULL, ylist1, P[id1].h.n, P[id2].lon, P[id2].lat, NULL, ylist2, P[id2].h.n, FALSE, &XC);
			GMT_free ((void *)ylist2);
			if (fabs (x_shift) > GMT_CONV_LIMIT) for (i = 0; i < P[id2].h.n; i++) P[id2].lon[i] -= x_shift;
			if (nx) {
				for (i = 0; i < nx; i++) printf ("%d\t%d\t%d\t%d\t%lf\t%lf\n", P[id1].h.id, P[id2].h.id, (int)floor(XC.xnode[0][i]), (int)floor(XC.xnode[1][i]), XC.x[i], XC.y[i]);
				GMT_x_free (&XC);
			}
			nx_tot += nx;
		}
		GMT_free ((void *)ylist1);
	}
	
	if (verbose) fprintf (stderr, "\npolygon_xover: %d external crossovers\n", nx_tot);

	exit(0);
}

int nothing_in_common (struct GMT3_POLY *hi, struct GMT3_POLY *hj, double *shift)
{
	double w, e;

	if (hi->north < hj->south || hi->south > hj->north) return (TRUE);

	w = hj->west - 360.0;	e = hj->east - 360.0;
	*shift = -360.0;
	while (e < hi->west) e += 360.0, w += 360.0, (*shift) += 360.0;
	if (w > hi->east) return (TRUE);
	return (FALSE);
}
