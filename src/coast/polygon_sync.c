/*
 *	$Id: polygon_sync.c,v 1.1 2009-06-11 21:07:58 guru Exp $
 * Based on output of polygon_hierarchy, update the h-i-l-c files with
 * meta data from the full set.
 */
#include "wvs.h"

#define FULL		0
#define NOT_PRESENT	-1

struct POLYGON {
	struct GMT3_POLY h;
	struct LONGPAIR *p;
} P[5][N_POLY];

int main (int argc, char **argv) {
	int i, n_id[5], id1, id2, res, father, *link[5], *level[5];
	char *kind = "fhilc", file[BUFSIZ], T[5][64], line[BUFSIZ];
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
		level[res] = (int *) GMT_memory (VNULL, n_id[FULL], sizeof (int), "polygon_hierarchy");
		for (id1 = 0; id1 < n_id[FULL]; id1++) link[res][id1] = NOT_PRESENT;
	}
	
	if ((fp = fopen ("GSHHS_hierarchy.txt", "r")) == NULL) {
		fprintf (stderr, "polygon_hierarchy: Cannot open hierarchy file\n");
		exit (EXIT_FAILURE);
	}
	for (id1 = 0; id1 < n_id[FULL]; id1++) {
		fgets (line, BUFSIZ, fp);
		sscanf (line, "%s %s %s %s %s", T[0], T[1], T[2], T[3], T[4]);
		for (res = 0; res < 5; res++) {
			for (i = 0; i < strlen(T[res]); i++) if (T[res][i] == '-') T[res][i] = ' ';
			sscanf (T[res], "%d %d", &link[res][id1], &level[res][id1]);
			if (res > 0 && link[res][id1] >= 0 && level[res][id1] != level[FULL][id1]) {
				fprintf (stderr, "ERROR: Siblings of pol %d differ in levels\n", link[FULL][id1]);
				exit (-1);
			}
		}
		
	}
	
	for (res = 1; res < 5; res++) {
		strcpy (file, argv[1]);	/* Get full file and replace _f with _<res> */
		for (i = 1; res > 0 && i < strlen(file); i++) if (file[i] == 'f' && file[i-1] == '_') file[i] = kind[res];
		fprintf (stderr, "Write new file %s\n", file);
		if ((fp = fopen (file, "w")) == NULL) {
			fprintf (stderr, "polygon_hierarchy: Cannot create file %s\n", file);
			exit (EXIT_FAILURE);
		}
		for (id2 = 0; id2 < n_id[res]; id2++) {
			for (id1 = 0, father = -1; id1 < n_id[FULL] && father == -1; id1++) if (link[res][id1] == id2) father = id1;	/* Get id to full version */
			P[res][id2].h.area = P[FULL][id1].h.area;
			P[res][id2].h.greenwich += (P[FULL][id1].h.id << 1);
			pol_writeheader (&P[res][id2].h, fp);
			if (pol_fwrite (P[res][id2].p, P[res][id2].h.n, fp) != P[res][id2].h.n) {
				fprintf(stderr,"polygon_xover:  ERROR  writing %d points from file %s.\n", P[res][id2].h.n, file);
				exit (EXIT_FAILURE);
			}
		}
		fclose (fp);
	}
		
	/* Free all polygons */
	for (res = 0; res < 5; res++) {
		for (id1 = 0; id1 < n_id[res]; id1++) GMT_free ((void *)P[res][id1].p);
	}
	for (res = 1; res < 5; res++) GMT_free ((void*)link[res]);
	for (res = 1; res < 5; res++) GMT_free ((void*)level[res]);
	
#ifdef DEBUG
	GMT_memtrack_on (GMT_mem_keeper);
#endif
	GMT_end (argc, argv);
	
	exit (EXIT_SUCCESS);
}	
