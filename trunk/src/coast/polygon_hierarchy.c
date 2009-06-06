/*
 *	$Id: polygon_hierarchy.c,v 1.2 2009-06-06 10:49:23 guru Exp $
 * Determines the polygon ID in the full resolution that corresponds to
 * the lower-resolution polygons.
 */
#include "wvs.h"

#define FULL		0
#define NOT_PRESENT	-1

struct POLYGON {
	struct GMT3_POLY h;
	struct LONGPAIR *p;
	int father;	/* Id of full resolution polygon */
} P[5][N_POLY];

int nothing_in_common (struct GMT3_POLY *hi, struct GMT3_POLY *hj, double *shift);

int main (int argc, char **argv) {
	int i, j, n_id[5], id1, id2, dx, res, go, ix_shift, in, *link[5];
	double x_shift = 0.0;
	char *kind = "fhilc", file[BUFSIZ];
	FILE *fp;
	
	argc = GMT_begin (argc, argv);
	
	if (argc == 1) {
		fprintf (stderr, "usage: polygon_hierarchy path-to-full.b\n");
		exit (EXIT_FAILURE);
	}
	
#ifdef DEBUG
	GMT_memtrack_off (GMT_mem_keeper);
#endif
	for (res = 0; res < 5; res++) {
		strcpy (file, argv[1]);	/* Get full file and replace _f with _<res> */
		for (i = 1; res > 0 && i < strlen(file); i++) if (file[i] == 'f' && file[i-1] == '_') file[i] = kind[res];
		fprintf (stderr, "Read file %s\n", file);
		if ((fp = fopen (file, "r")) == NULL) {
			fprintf (stderr, "polygon_hierarchy: Cannot open file %s\n", file);
			exit (EXIT_FAILURE);
		}
		n_id[res] = 0;
		while (pol_readheader (&P[res][n_id[res]].h, fp) == 1) {
			P[res][n_id[res]].father = NOT_PRESENT;	/* Initially we do not know the father */
			P[res][n_id[res]].p = (struct LONGPAIR *) GMT_memory (VNULL, P[res][n_id[res]].h.n, sizeof (struct LONGPAIR), "polygon_hierarchy");
			if (pol_fread (P[res][n_id[res]].p, P[res][n_id[res]].h.n, fp) != P[res][n_id[res]].h.n) {
				fprintf(stderr,"polygon_xover:  ERROR  reading %d points from file %s.\n", P[res][n_id[res]].h.n, file);
				exit (EXIT_FAILURE);
			}
			n_id[res]++;
		}
		fclose (fp);
	}
	
	for (res = 1; res < 5; res++) {	/* Initialize all parents to NONE */
		link[res] = (int *) GMT_memory (VNULL, n_id[FULL], sizeof (int), "polygon_hierarchy");
		for (id1 = 0; id1 < n_id[FULL]; id1++) link[res][id1] = NOT_PRESENT;
	}
	
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
	
	for (id1 = 0; id1 < n_id[FULL]; id1++) {	/* For all full resolution polygons */
	
		fprintf (stderr, "Full polygon %7d res f", P[FULL][id1].h.id);
		for (res = 1; res < 5; res++) {	/* For each of the lower resolutions */
			fprintf (stderr, "-%c", kind[res]);
			for (id2 = 0; link[res][id1] == NOT_PRESENT && id2 < n_id[res]; id2++) {
			
				if (P[res][id2].father >= 0) continue;	/* Already determined the father polygon */
				if (nothing_in_common (&P[FULL][id1].h, &P[res][id2].h, &x_shift)) continue;	/* No area in common */
				ix_shift = irint (x_shift) * MILL;
			
				if (P[FULL][id1].h.id == 0) {	/* Check if a point (any point, really) is outside the current crude outside AFREUR polygon */
					if ((in = non_zero_winding2 (P[res][id2].p[0].x + ix_shift, P[res][id2].p[0].y, ieur_o[0], ieur_o[1], N_EUR_O)) == 0) continue;	/* Polygon id2 completely outside the "outside" polygon */
					if ((in = non_zero_winding2 (P[res][id2].p[0].x + ix_shift, P[res][id2].p[0].y, iafr_i[0], iafr_i[1], N_AFR_I)) == 2) continue;	/* Polygon id2 completely outside the "outside" polygon */
					if ((in = non_zero_winding2 (P[res][id2].p[0].x + ix_shift, P[res][id2].p[0].y, ieur_i[0], ieur_i[1], N_EUR_I)) == 2) continue;	/* Polygon id2 completely inside the "inside" polygon 1 */
				}
				else if (P[FULL][id1].h.id == 1) {	/* Check if inside the first of possibly two crude polgons */
					if ((in = non_zero_winding2 (P[res][id2].p[0].x + ix_shift, P[res][id2].p[0].y, iam_o[0],  iam_o[1],  N_AM_O))  == 0) continue;	/* Polygon id2 completely inside the "inside" polygon 1 */
					if ((in = non_zero_winding2 (P[res][id2].p[0].x + ix_shift, P[res][id2].p[0].y, inam_i[0], inam_i[1], N_NAM_I)) == 2) continue;	/* Polygon id2 completely inside the "inside" polygon 1 */
					if ((in = non_zero_winding2 (P[res][id2].p[0].x + ix_shift, P[res][id2].p[0].y, isam_i[0], isam_i[1], N_SAM_I)) == 2) continue;	/* Polygon id2 completely inside the "inside" polygon 1 */
				}
				else if (P[FULL][id1].h.id == 3) {	/* Check if inside the 2nd crude polgon */
					if ((in = non_zero_winding2 (P[res][id2].p[0].x + ix_shift, P[res][id2].p[0].y, iaus_o[0], iaus_o[1], N_AUS_O)) == 0) continue;	/* Polygon id2 completely inside the "inside" polygon 1 */
					if ((in = non_zero_winding2 (P[res][id2].p[0].x + ix_shift, P[res][id2].p[0].y, iaus_i[0], iaus_i[1], N_AUS_I)) == 2) continue;	/* Polygon id2 completely inside the "inside" polygon 1 */
				}
				for (i = 0, go = TRUE; go && i < P[FULL][id1].h.n; i++) {	/* For all points on full polygon */
					for (j = 0; go && j < P[res][id2].h.n; j++) {		/* For all points on the lower-resolution polygon */
						if (P[FULL][id1].p[i].y != P[res][id2].p[j].y) continue;	/* No match */
						dx = P[FULL][id1].p[i].x - P[res][id2].p[j].x;
						if (dx == 0 || dx == M360 || dx == -M360) go = FALSE;		/* Matching point */
					}
				}
				if (go) continue;	/* Pol id1 not found in this resolution */
				/* OK, found a match, set father nad link to end id2 loop */
				P[res][id2].father = P[FULL][id1].h.id;
				link[res][id1] = P[res][id2].h.id;
			}
		}
		fprintf (stderr, "\n");
	}
		
	fprintf (stderr, "\nWrite hierarchy table\n");
	
	if ((fp = fopen ("GSHHS_hierarchy.txt", "w")) == NULL) {
		fprintf (stderr, "polygon_hierarchy: Cannot open hierarchy fil\n");
		exit (EXIT_FAILURE);
	}
	for (id1 = 0; id1 < n_id[FULL]; id1++) {
		fprintf (fp, "%d", P[FULL][id1].h.id);
		for (res = 1; res < 5; res++) fprintf (fp, "\t%d", link[res][id1]);
		fprintf (fp, "\t%g\t%d\t%d\n", P[FULL][id1].h.area, P[FULL][id1].h.parent, P[FULL][id1].h.river);
	}
	fclose (fp);
	
	/* Free all polygons */
	for (res = 0; res < 5; res++) {
		for (id1 = 0; id1 < n_id[res]; id1++) GMT_free ((void *)P[res][id1].p);
	}
	for (res = 1; res < 5; res++) GMT_free ((void*)link[res]);
	
#ifdef DEBUG
	GMT_memtrack_on (GMT_mem_keeper);
#endif
	GMT_end (argc, argv);
	
	exit (EXIT_SUCCESS);
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
