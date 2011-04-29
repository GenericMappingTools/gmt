/*
 *	$Id: polygon_hierarchy.c,v 1.11 2011-04-29 03:08:12 guru Exp $
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

int main (int argc, char **argv) {
	int i, j, n_id[5], id1, id2, dx, res, go, ix_shift, c, in, cont_no_1, cont_no_2, *link[5], *level[5];
	int *IX[N_CONTINENTS][2], *IY[N_CONTINENTS][2], N[N_CONTINENTS][2];
	double x_shift = 0.0;
	char *kind = "fhilc", file[GMT_BUFSIZ];
	FILE *fp;
	
	argc = GMT_begin (argc, argv);
	
	if (argc == 1) {
		fprintf (stderr, "usage: polygon_hierarchy path-to-full.b\n");
		exit (EXIT_FAILURE);
	}
	
#ifdef DEBUG
	GMT_memtrack_off (GMT->dbg.mem_keeper);
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
				fprintf(stderr,"polygon_xover: Error reading %d points from file %s.\n", P[res][n_id[res]].h.n, file);
				exit (EXIT_FAILURE);
			}
			n_id[res]++;
		}
		fclose (fp);
	}
	
	for (res = 1; res < 5; res++) {	/* Initialize all parents to NONE */
		link[res] = (int *) GMT_memory (VNULL, n_id[FULL], sizeof (int), "polygon_hierarchy");
		level[res] = (int *) GMT_memory (VNULL, n_id[FULL], sizeof (int), "polygon_hierarchy");
		for (id1 = 0; id1 < n_id[FULL]; id1++) link[res][id1] = NOT_PRESENT;
		for (id1 = 0; id1 < N_CONTINENTS; id1++) level[res][id1] = 1;	/* Since not set in loop below */
	}
	
	/* Scale crude polygons by 1e6 to match the data scale */
	
	crude_init_int (IX, IY, N, MILL);

	fprintf (stderr, "Start comparisons\n\n");
	
	for (id1 = 0; id1 < n_id[FULL]; id1++) {	/* For all full resolution polygons */
	
		fprintf (stderr, "Full polygon %7d res f", P[FULL][id1].h.id);
		cont_no_1 = (P[FULL][id1].h.river >> 8);	/* Get continent number 1-6 (0 if not a continent) */
		for (res = 1; res < 5; res++) {	/* For each of the lower resolutions */
			fprintf (stderr, "-%c", kind[res]);
			for (id2 = 0; link[res][id1] == NOT_PRESENT && id2 < n_id[res]; id2++) {
				if (P[res][id2].father >= 0) continue;	/* Already determined the father polygon */
				cont_no_2 = (P[res][id2].h.river >> 8);	/* Get continent nubmer 1-6 (0 if not a continent) */
				if (cont_no_1 && cont_no_2) continue;	/* Skip pairs of continents as they cannot contain each other */
				if (nothing_in_common (&P[FULL][id1].h, &P[res][id2].h, &x_shift)) continue;	/* No area in common */
				ix_shift = irint (x_shift) * MILL;
				if (cont_no_1 > 0 && cont_no_1 != ANTARCTICA ) {	/* Any continent other than Antarctica */
					c = cont_no_1 - 1;
					if ((in = non_zero_winding2 (P[res][id2].p[0].x + ix_shift, P[res][id2].p[0].y, IX[c][OUTSIDE], IY[c][OUTSIDE], N[c][OUTSIDE])) == 0) continue;	/* Polygon id2 completely outside the "outside" polygon */
					if ((in = non_zero_winding2 (P[res][id2].p[0].x + ix_shift, P[res][id2].p[0].y, IX[c][INSIDE], IY[c][INSIDE], N[c][INSIDE])) == 2) continue;	/* Polygon id2 completely inside the "inside" polygon */
				}
				/* Here we must do full check */
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
				level[res][id1] = P[res][id2].h.level;
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
		fprintf (fp, "%d\t%d", P[FULL][id1].h.id, P[FULL][id1].h.level);
		for (res = 1; res < 5; res++) fprintf (fp, "\t%d\t%d", link[res][id1], level[res][id1]);
		fprintf (fp, "\t%g\t%d\t%d\t%.6f\t%.6f\t%.6f\t%.6f\t%d\n", P[FULL][id1].h.area, P[FULL][id1].h.parent, P[FULL][id1].h.river, \
		P[FULL][id1].h.west, P[FULL][id1].h.east, P[FULL][id1].h.south, P[FULL][id1].h.north, P[FULL][id1].h.source);
	}
	fclose (fp);
	
	/* Free all polygons */
	for (res = 0; res < 5; res++) {
		for (id1 = 0; id1 < n_id[res]; id1++) GMT_free ((void *)P[res][id1].p);
	}
	for (res = 1; res < 5; res++) GMT_free ((void*)link[res]);
	for (res = 1; res < 5; res++) GMT_free ((void*)level[res]);
	crude_free_int (IX, IY, N);
	
#ifdef DEBUG
	GMT_memtrack_on (GMT->dbg.mem_keeper);
#endif
	GMT_end (argc, argv);
	
	exit (EXIT_SUCCESS);
}	
