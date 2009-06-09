/*
 *	$Id: polygon_xover.c,v 1.15 2009-06-09 02:26:02 guru Exp $
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

int main (int argc, char **argv)
{
	FILE	*fp;
	int i, n_id, id1, id2, nx, nx_tot, verbose, cnt, full, fast, in, eur_id = 0, cont_no, N[5][2];
	double x_shift = 0.0, r, theta, c, s, *X, *Y, *CX[5][2], *CY[5][2];
	struct GMT_XSEGMENT *ylist1, *ylist2;
	struct GMT_XOVER XC;
	struct LONGPAIR p;

	if (argc < 2 || argc > 3) {
		fprintf(stderr,"usage:  polygon_xover wvs_polygons.b [-V] > report.lis\n");
		exit(-1);
	}

	/* Open file and read everything into memory */
	
	if ((fp = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr,"polygon_xover: Could not open file %s\n", argv[1]);
		exit(-1);
	}
	verbose = (argc == 3);
	n_id = 0;
	while (pol_readheader (&P[n_id].h, fp) == 1) {
		P[n_id].lon = (double *) GMT_memory (VNULL, P[n_id].h.n, sizeof (double), "polygon_xover");
		P[n_id].lat = (double *) GMT_memory (VNULL, P[n_id].h.n, sizeof (double), "polygon_xover");
		if (P[n_id].h.east < 0.0 && P[n_id].h.west< 0.0) {
			fprintf (stderr, "Pol %d has negative w/e values.  Run polygon_fixnegwesn\n", P[n_id].h.id);
			exit(-1);
		}
		cont_no = (P[n_id].h.river >> 8);	/* Get continent nubmer 1-6 (0 if not a continent) */
		if (cont_no == EURASIA) eur_id = n_id;	/* blob with Eurasia */

		for (i = 0; i < P[n_id].h.n; i++) {
			if (pol_fread (&p, 1, fp) != 1) {
				fprintf(stderr,"polygon_xover:  ERROR  reading file.\n");
				exit(-1);
			}
			if (P[n_id].h.greenwich && p.x > P[n_id].h.datelon) p.x -= M360;
			P[n_id].lon[i] = p.x * 1e-6;
			P[n_id].lat[i] = p.y * 1e-6;
		}
		/* lon,lat is now -180/+270 and continuous */
		if (cont_no == ANTARCTICA) {	/* Special r,theta conversion */
			for (i = 0; i < P[n_id].h.n; i++) {
				sincosd (P[n_id].lon[i], &s, &c);
				r = P[n_id].lat[i] + 90.0;	/* So r = 0 is S pole */
				P[n_id].lon[i] = r * c;
				P[n_id].lat[i] = r * s;
			}
		}

		n_id++;
	}
	fclose(fp);
	if (verbose) fprintf (stderr, "polygon_xover: Found %d polygons\n", n_id);
	
	crude_init (CX, CY, N);
	
	/* Now do double do-loop to find all xovers */
	
	full = (P[eur_id].h.n > 1000000);	/* Only the full resolution has more than 1 mill points for EUR-AFR polygon */
	
	nx_tot = 0;
	for (id1 = 0; id1 < n_id; id1++) {
		if (id1 == 0) continue;
		cont_no = (P[id1].h.river >> 8);	/* Get continent nubmer 1-6 (0 if not a continent) */
		
		GMT_init_track (P[id1].lat, P[id1].h.n, &ylist1);
			
		for (id2 = MAX (6, id1 + 1); id2 < n_id; id2++) {	/* Dont start earlier than 4 since no point comparing continents */
			if (cont_no == ANTARCTICA) {	/* Must compare using r,theta */
				if (P[id2].h.south > P[id1].h.north) continue;	/* Too far north */
				X = (double *) GMT_memory (VNULL, P[id2].h.n, sizeof (double), "polygon_xover");
				Y = (double *) GMT_memory (VNULL, P[id2].h.n, sizeof (double), "polygon_xover");
				for (i = 0; i < P[id2].h.n; i++) {
					sincosd (P[id2].lon[i], &s, &c);
					r = P[id2].lat[i] + 90.0;	/* So r = 0 is S pole */
					X[i] = r * c;
					Y[i] = r * s;
				}
			}
			else {	/* Regular Cartesian testing */
		
				if (nothing_in_common (&P[id1].h, &P[id2].h, &x_shift)) continue;	/* No area in common */

				/* GMT_non_zero_winding returns 2 if inside, 1 if on line, and 0 if outside */
			
				if (fast && cont_no) {	/* Use course outlines to determine if id2 is inside/outside a continent */
					cnt = cont_no - 1;
					for (i = in = 0; i < P[id2].h.n; i++) in += GMT_non_zero_winding (P[id2].lon[i] + x_shift, P[id2].lat[i], CX[cnt][OUTSIDE], CY[cnt][OUTSIDE], N[cnt][OUTSIDE]);
					if (in == 0) continue;	/* Polygon id2 completely outside the "outside" polygon */
					for (i = in = 0; i < P[id2].h.n; i++) in += GMT_non_zero_winding (P[id2].lon[i] + x_shift, P[id2].lat[i], CX[cnt][INSIDE], CY[cnt][INSIDE], N[cnt][INSIDE]);
					if (in == (2 * P[id2].h.n)) continue;	/* Polygon id2 completely inside the "inside" polygon 1 */
				}
				X = P[id2].lon;
				Y = P[id2].lat;
				if (!GMT_IS_ZERO (x_shift)) for (i = 0; i < P[id2].h.n; i++) P[id2].lon[i] += x_shift;
			}
			
			/* Get here when no cheap determination worked and we must do full crossover calculation */
			
			if (verbose) fprintf (stderr, "polygon_xover: %6d vs %6d [T = %6d]\r", P[id1].h.id, P[id2].h.id, nx_tot);
			
			GMT_init_track (Y, P[id2].h.n, &ylist2);

			nx = GMT_crossover (P[id1].lon, P[id1].lat, NULL, ylist1, P[id1].h.n, X, Y, NULL, ylist2, P[id2].h.n, FALSE, &XC);
			GMT_free ((void *)ylist2);
			if (id1 == ANTARCTICA) {	/* Undo projection for crossover results */
				for (i = 0; i < nx; i++) {
					r = hypot (XC.x[i], XC.y[i]);
					theta = atan2d (XC.y[i], XC.x[i]);
					if (theta < 0.0) theta += 360.0;
					XC.x[i] = theta;
					XC.y[i] = r - 90.0;
				}
				GMT_free ((void *)X);
				GMT_free ((void *)Y);
			}
			else if (!GMT_IS_ZERO (x_shift)) {
				for (i = 0; i < P[id2].h.n; i++) P[id2].lon[i] -= x_shift;
			}
			if (nx) {
				for (i = 0; i < nx; i++) printf ("%d\t%d\t%d\t%d\t%f\t%f\n", P[id1].h.id, P[id2].h.id, (int)floor(XC.xnode[0][i]), (int)floor(XC.xnode[1][i]), XC.x[i], XC.y[i]);
				GMT_x_free (&XC);
			}
			nx_tot += nx;
		}
		GMT_free ((void *)ylist1);
	}
	crude_free (CX, CY, N);
	
	if (verbose) fprintf (stderr, "\npolygon_xover: %d external crossovers\n", nx_tot);

	exit(0);
}
