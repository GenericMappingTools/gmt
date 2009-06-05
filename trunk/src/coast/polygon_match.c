/*
 *	$Id: polygon_match.c,v 1.1 2009-06-05 04:10:16 guru Exp $
 * Compares the enw and old *.b files and looks for differences.
 */
#include "wvs.h"

#define NOT_PRESENT	-1

struct GMT3_POLY_OLD {
	int id;
	int n;
	int greenwich;  /* Greenwich is TRUE if Greenwich is crossed */
	int level;      /* -1 undecided, 0 ocean, 1 land, 2 lake, 3 island_in_lake, etc */
	int datelon;    /* 180 for all except eurasia (270) */
	int checked[2]; /* TRUE if polygon has been crossover checked with all peers */
	int source;     /* 0 = CIA WDBII, 1 = WVS */
	double west, east, south, north;
	double area;    /* Area of polygon */
};

struct POLYGON_NEW {
	struct GMT3_POLY h;
	struct LONGPAIR *p;
	int brother;	/* Id of matching polygon */
};

struct POLYGON_OLD {
	struct GMT3_POLY_OLD h;
	struct LONGPAIR *p;
	int brother;	/* Id of matching polygon */
};

int nothing_in_common (struct GMT3_POLY *hi, struct GMT3_POLY_OLD *hj, double *shift);
int pol_readheader_old (struct GMT3_POLY_OLD *h, FILE *fp);
#if WORDS_BIGENDIAN == 0
void swab_polheader_old (struct GMT3_POLY_OLD *h);
#endif

int main (int argc, char **argv) {
	int i, j, n_A = 0, n_B = 0, id1, id2, dx, go, ix_shift, in, n, j0;
	double x_shift = 0.0, f;
	char file[BUFSIZ], alarm[32];
	FILE *fp;
	struct POLYGON_NEW *new_P = NULL;
	struct POLYGON_OLD *old_P = NULL;
	
	argc = GMT_begin (argc, argv);
	
	if (argc == 1) {
		fprintf (stderr, "usage: polygon_match new.b old.b\n");
		exit (EXIT_FAILURE);
	}
	
#ifdef DEBUG
	GMT_memtrack_off (GMT_mem_keeper);
#endif

	new_P = (struct POLYGON_NEW *) GMT_memory (VNULL, N_POLY, sizeof (struct POLYGON_NEW), "polygon_match");
	old_P = (struct POLYGON_OLD *) GMT_memory (VNULL, N_POLY, sizeof (struct POLYGON_OLD), "polygon_match");
	
	/* Read new file */
	strcpy (file, argv[1]);
	fprintf (stderr, "Read new file %s\n", file);
	if ((fp = fopen (file, "r")) == NULL) {
		fprintf (stderr, "polygon_match: Cannot open file %s\n", file);
		exit (EXIT_FAILURE);
	}
	while (pol_readheader (&new_P[n_A].h, fp) == 1) {
		new_P[n_A].brother = NOT_PRESENT;	/* Initially we do not know the match */
		new_P[n_A].p = (struct LONGPAIR *) GMT_memory (VNULL, new_P[n_A].h.n, sizeof (struct LONGPAIR), "polygon_match");
		if (pol_fread (new_P[n_A].p, new_P[n_A].h.n, fp) != new_P[n_A].h.n) {
			fprintf(stderr,"polygon_match:  ERROR  reading %d points from file %s.\n", new_P[n_A].h.n, file);
			exit (EXIT_FAILURE);
		}
		n_A++;
	}
	fclose (fp);
	
	/* Read old file */
	strcpy (file, argv[2]);
	fprintf (stderr, "Read old file %s\n", file);
	if ((fp = fopen (file, "r")) == NULL) {
		fprintf (stderr, "polygon_match: Cannot open file %s\n", file);
		exit (EXIT_FAILURE);
	}
	while (pol_readheader_old (&old_P[n_B].h, fp) == 1) {
		old_P[n_B].brother = NOT_PRESENT;	/* Initially we do not know the match */
		old_P[n_B].p = (struct LONGPAIR *) GMT_memory (VNULL, old_P[n_B].h.n, sizeof (struct LONGPAIR), "polygon_match");
		if (pol_fread2 (old_P[n_B].p, old_P[n_B].h.n, fp) != old_P[n_B].h.n) {
			fprintf(stderr,"polygon_match:  ERROR  reading %d points from file %s.\n", old_P[n_B].h.n, file);
			exit (EXIT_FAILURE);
		}
		n_B++;
	}
	fclose (fp);
	
	/* Scale crude polygons by 1e6 to match the data scale */
	
	for (i = 0; i < N_EUR_O; i++) {
		ieur_o[0][i] = (ieur_o[0][i] - 360) * MILL;
		ieur_o[1][i] *= MILL;
	}
	for (i = 0; i < N_EUR_I; i++) {
		ieur_i[0][i] = (ieur_i[0][i] - 360) * MILL;
		ieur_i[1][i] *= MILL;
	}
	for (i = 0; i < N_AFR_I; i++) {
		iafr_i[0][i] = (iafr_i[0][i] - 360) * MILL;
		iafr_i[1][i] *= MILL;
	}
	for (i = 0; i < N_AM_O; i++) {
		iam_o[0][i] *= MILL;
		iam_o[1][i] *= MILL;
	}
	for (i = 0; i < N_SAM_I; i++) {
		isam_i[0][i] *= MILL;
		isam_i[1][i] *= MILL;
	}
	for (i = 0; i < N_NAM_I; i++) {
		inam_i[0][i] *= MILL;
		inam_i[1][i] *= MILL;
	}
	for (i = 0; i < N_AUS_O; i++) {
		iaus_o[0][i] *= MILL;
		iaus_o[1][i] *= MILL;
	}
	for (i = 0; i < N_AUS_I; i++) {
		iaus_i[0][i] *= MILL;
		iaus_i[1][i] *= MILL;
	}

	fprintf (stderr, "Start comparisons\n\n");
	
	for (id1 = 0; id1 < n_A; id1++) {	/* For all polygons in the new file*/
	
		/* if (id1%10 == 0) fprintf (stderr, "Polygon %d\r", id1); */

		for (id2 = 0; new_P[id1].brother == NOT_PRESENT && id2 < n_B; id2++) {	/* For all polygons in the old file*/
			if (old_P[id2].brother >= 0) continue;	/* Already determined to match another polygon */
			if (nothing_in_common (&new_P[id1].h, &old_P[id2].h, &x_shift)) continue;	/* No area in common */
			ix_shift = irint (x_shift) * MILL;
		
			if (id1 == 0) {	/* Check if a point (any point, really) is outside the current crude outside AFREUR polygon */
				if ((in = non_zero_winding2 (old_P[id2].p[0].x + ix_shift, old_P[id2].p[0].y, ieur_o[0], ieur_o[1], N_EUR_O)) == 0) continue;	/* Polygon id2 completely outside the "outside" polygon */
				if ((in = non_zero_winding2 (old_P[id2].p[0].x + ix_shift, old_P[id2].p[0].y, iafr_i[0], iafr_i[1], N_AFR_I)) == 2) continue;	/* Polygon id2 completely outside the "outside" polygon */
				if ((in = non_zero_winding2 (old_P[id2].p[0].x + ix_shift, old_P[id2].p[0].y, ieur_i[0], ieur_i[1], N_EUR_I)) == 2) continue;	/* Polygon id2 completely inside the "inside" polygon 1 */
			}
			else if (id1 == 1) {	/* Check if inside the first of possibly two crude polgons */
				if ((in = non_zero_winding2 (old_P[id2].p[0].x + ix_shift, old_P[id2].p[0].y, iam_o[0],  iam_o[1],  N_AM_O))  == 0) continue;	/* Polygon id2 completely inside the "inside" polygon 1 */
				if ((in = non_zero_winding2 (old_P[id2].p[0].x + ix_shift, old_P[id2].p[0].y, inam_i[0], inam_i[1], N_NAM_I)) == 2) continue;	/* Polygon id2 completely inside the "inside" polygon 1 */
				if ((in = non_zero_winding2 (old_P[id2].p[0].x + ix_shift, old_P[id2].p[0].y, isam_i[0], isam_i[1], N_SAM_I)) == 2) continue;	/* Polygon id2 completely inside the "inside" polygon 1 */
			}
			else if (id1 == 3) {	/* Check if inside the 2nd crude polgon */
				if ((in = non_zero_winding2 (old_P[id2].p[0].x + ix_shift, old_P[id2].p[0].y, iaus_o[0], iaus_o[1], N_AUS_O)) == 0) continue;	/* Polygon id2 completely inside the "inside" polygon 1 */
				if ((in = non_zero_winding2 (old_P[id2].p[0].x + ix_shift, old_P[id2].p[0].y, iaus_i[0], iaus_i[1], N_AUS_I)) == 2) continue;	/* Polygon id2 completely inside the "inside" polygon 1 */
			}
			for (i = n = 0, go = TRUE; go && i < new_P[id1].h.n; i++) {	/* For all points on A polygon */
				for (j = 0; go && j < old_P[id2].h.n; j++, n++) {		/* For all points on the B polygon */
					if (new_P[id1].p[i].y != old_P[id2].p[j].y) continue;	/* Not a match */
					dx = new_P[id1].p[i].x - old_P[id2].p[j].x;
					if (dx == 0 || dx == M360 || dx == -M360) go = FALSE;		/* Found a matching point, stop the loops */
				}
			}
			if (go) continue;	/* Pol id2 not a match for Pol id1 */
			
			/* OK, polygon B shares at least one point with A so likely the same polygon.  Determine level of matching */
			
			new_P[id1].brother = id2;
			old_P[id2].brother = id1;
			memset ((void *)alarm, 0, 32);
			if (new_P[id1].h.id != old_P[id2].h.id) strcat (alarm, " I");			/* I = Id mismatch */
			if (new_P[id1].h.n != old_P[id2].h.n) strcat (alarm, " #");			/* # = n mismatch */
			if (new_P[id1].h.greenwich != old_P[id2].h.greenwich) strcat (alarm, " G");	/* G = greenwich mismatch */
			if (new_P[id1].h.level != old_P[id2].h.level) strcat (alarm, " L");		/* L = level mismatch */
			if (new_P[id1].h.datelon != old_P[id2].h.datelon) strcat (alarm, " D");		/* D = datelon mismatch */
			if (new_P[id1].h.checked[0] != old_P[id2].h.checked[0]) strcat (alarm, " 1");	/* 1 = checked[0] mismatch */
			if (new_P[id1].h.checked[1] != old_P[id2].h.checked[1]) strcat (alarm, " 2");	/* 2 = checked[1] mismatch */
			if (new_P[id1].h.source != old_P[id2].h.source) strcat (alarm, " O");		/* O = source (origin) mismatch */
			if (new_P[id1].h.west != old_P[id2].h.west) strcat (alarm, " W");		/* W = West mismatch */
			if (new_P[id1].h.east != old_P[id2].h.east) strcat (alarm, " E");		/* E = East mismatch */
			if (new_P[id1].h.south != old_P[id2].h.south) strcat (alarm, " S");		/* S = South mismatch */
			if (new_P[id1].h.north != old_P[id2].h.north) strcat (alarm, " N");		/* N = North mismatch */
			if (new_P[id1].h.area > 0.0 && old_P[id2].h.area > 0.0) {
				f = fabs ((new_P[id1].h.area / old_P[id2].h.area) - 1.0) * 1e6;	/* ppm change */
				if (f >= 1.0) strcat (alarm, " A");					/* A = area mismatch exceeding 1 ppm */
			}
			if (new_P[id1].h.n == old_P[id2].h.n && n == 1) {	/* Same number of points and matched one first point, will need to check additional points */
				for (i = n = j0 = 0; n == i && i < new_P[id1].h.n; i++) {	/* For all points on A polygon */
					for (j = j0, go = TRUE; go && j < old_P[id2].h.n; j++) {	/* For all points on the B polygon */
						if (new_P[id1].p[i].y != old_P[id2].p[j].y) continue;	/* No match */
						dx = new_P[id1].p[i].x - old_P[id2].p[j].x;
						if (!(dx == 0 || dx == M360 || dx == -M360)) continue;	/* No match */
						go = FALSE;	/* Found a match so stop the inner loop */
						if (j0 == i) j0++;	/* Move j-start up one since we are done with first point */
					}
					if (!go) n++;		/* n increments when a matching point is found for point i */
				}
				if (n < i) strcat (alarm, " P");					/* P = Point mismatch */
			}
			else if (n > 1)	/* More than one point differs */
				strcat (alarm, " P");							/* P = Point mismatch */
			if (alarm[0]) printf ("New %d vs old %d :%s\n", id1, id2, alarm);
		}
	}
	
	/* Free all polygons */
	for (id1 = 0; id1 < n_A; id1++) {
		if (new_P[id1].brother == NOT_PRESENT) printf ("New %d not in old\n", id1);
		GMT_free ((void *)new_P[id1].p);
	}
	for (id2 = 0; id2 < n_B; id2++) {
		if (old_P[id2].brother == NOT_PRESENT) printf ("Old %d not in new\n", id2);
		GMT_free ((void *)old_P[id2].p);
	}
	GMT_free ((void *)new_P);
	GMT_free ((void *)old_P);
	
#ifdef DEBUG
	GMT_memtrack_on (GMT_mem_keeper);
#endif
	GMT_end (argc, argv);
	
	exit (EXIT_SUCCESS);
}	

int pol_readheader_old (struct GMT3_POLY_OLD *h, FILE *fp)
{
	int n;
	n = fread ((void *)h, sizeof (struct GMT3_POLY_OLD), 1, fp);
#if WORDS_BIGENDIAN == 0
	swab_polheader_old (h);
#endif
	return (n);
}

#if WORDS_BIGENDIAN == 0
void swab_polheader_old (struct GMT3_POLY_OLD *h)
{
	unsigned int *i, j;

	h->id = GMT_swab4 (h->id);
	h->n = GMT_swab4 (h->n);
	h->greenwich = GMT_swab4 (h->greenwich);
	h->level = GMT_swab4 (h->level);
	h->datelon = GMT_swab4 (h->datelon);
	h->checked[0] = GMT_swab4 (h->checked[0]);
	h->checked[1] = GMT_swab4 (h->checked[1]);
	h->source = GMT_swab4 (h->source);
	i = (unsigned int *)&h->west;
	j = GMT_swab4 (i[0]);
	i[0] = GMT_swab4 (i[1]);
	i[1] = j;
	i = (unsigned int *)&h->east;
	j = GMT_swab4 (i[0]);
	i[0] = GMT_swab4 (i[1]);
	i[1] = j;
	i = (unsigned int *)&h->south;
	j = GMT_swab4 (i[0]);
	i[0] = GMT_swab4 (i[1]);
	i[1] = j;
	i = (unsigned int *)&h->north;
	j = GMT_swab4 (i[0]);
	i[0] = GMT_swab4 (i[1]);
	i[1] = j;
	i = (unsigned int *)&h->area;
	j = GMT_swab4 (i[0]);
	i[0] = GMT_swab4 (i[1]);
	i[1] = j;
}
#endif

int nothing_in_common (struct GMT3_POLY *hi, struct GMT3_POLY_OLD *hj, double *shift)
{
	double w, e;

	if (hi->north < hj->south || hi->south > hj->north) return (TRUE);

	w = hj->west - 360.0;	e = hj->east - 360.0;
	*shift = -360.0;
	while (e < hi->west) e += 360.0, w += 360.0, (*shift) += 360.0;
	if (w > hi->east) return (TRUE);
	return (FALSE);
}
