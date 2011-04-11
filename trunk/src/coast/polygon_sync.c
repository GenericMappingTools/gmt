/*
 *	$Id: polygon_sync.c,v 1.6 2011-04-11 21:15:32 remko Exp $
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
	int i, n_id[5], id1, id2, res, father, error = 0, *link[5], *level[5];
	char *kind = "fhilc", file[BUFSIZ], line[BUFSIZ];
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
			P[res][n_id[res]].p = (struct LONGPAIR *) GMT_memory (VNULL, P[res][n_id[res]].h.n, sizeof (struct LONGPAIR), "polygon_hierarchy");
			if (pol_fread (P[res][n_id[res]].p, P[res][n_id[res]].h.n, fp) != P[res][n_id[res]].h.n) {
				fprintf(stderr,"polygon_xover:  Error reading %d points from file %s.\n", P[res][n_id[res]].h.n, file);
				exit (EXIT_FAILURE);
			}
			n_id[res]++;
		}
		fclose (fp);
	}
	
	for (res = 0; res < 5; res++) {	/* Initialize all parents to NONE */
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
		sscanf (line, "%d %d %d %d %d %d %d %d %d %d ", &link[0][id1], &level[0][id1], &link[1][id1], &level[1][id1], &link[2][id1], &level[2][id1], &link[3][id1], &level[3][id1], &link[4][id1], &level[4][id1]);
		for (res = 0; res < 5; res++) {
			if (res > 0 && link[res][id1] >= 0 && level[res][id1] != level[FULL][id1]) {
				fprintf (stderr, "Error: Siblings of pol %d differ in levels\n", link[FULL][id1]);
				error++;
				printf ("%s", line);
			}
			if (res > 0 && link[res][id1] >= 0 && link[res-1][id1] == -1) {
				fprintf (stderr, "Error: Siblings of pol %d missing in a higher resolution\n", link[FULL][id1]);
				error++;
				printf ("%s", line);
			}
		}
		
	}
	if (error) exit (-1);
	
	for (res = 1; res < 5; res++) {
		fprintf (stderr, "Test links for res %c\n", kind[res]);
		for (id2 = 0; id2 < n_id[res]; id2++) {
			if (id2 < N_CONTINENTS)
				father = id2;
			else {
				for (id1 = N_CONTINENTS, father = -1; id1 < n_id[FULL] && father == -1; id1++) if (link[res][id1] == id2) father = id1;	/* Get id to full version */
			}
			if (father == -1) {
				printf ("Error: Cannot find father of %c id = %d!\n", kind[res], id2);
				error++;
			}
			else if (P[res][id2].h.river != P[FULL][father].h.river) {
				printf ("Error: Sibling of river pol %d has wrong river value (%d) for %c id = %d\n", father, P[res][id2].h.river, kind[res], id2);
				error++;
			}
		}
	}
	if (error) exit (-1);

	for (res = 1; res < 5; res++) {
		strcpy (file, argv[1]);	/* Get full file and replace _f with _<res> */
		for (i = 1; res > 0 && i < strlen(file); i++) if (file[i] == 'f' && file[i-1] == '_') file[i] = kind[res];
		fprintf (stderr, "Write new file %s\n", file);
		if ((fp = fopen (file, "w")) == NULL) {
			fprintf (stderr, "polygon_hierarchy: Cannot create file %s\n", file);
			exit (EXIT_FAILURE);
		}
		for (id2 = 0; id2 < n_id[res]; id2++) {
			if (id2 < N_CONTINENTS)
				father = id2;
			else {
				for (id1 = N_CONTINENTS, father = -1; id1 < n_id[FULL] && father == -1; id1++) if (link[res][id1] == id2) father = id1;	/* Get id to full version */
			}
			P[res][id2].h.area = P[FULL][father].h.area;
			P[res][id2].h.west = P[FULL][father].h.west;
			P[res][id2].h.east = P[FULL][father].h.east;
			P[res][id2].h.south = P[FULL][father].h.south;
			P[res][id2].h.north = P[FULL][father].h.north;
			P[res][id2].h.datelon = P[FULL][father].h.datelon;
			P[res][id2].h.source = P[FULL][father].h.source;
			P[res][id2].h.greenwich += (P[FULL][father].h.id << 1);
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
	GMT_memtrack_on (GMT->dbg.mem_keeper);
#endif
	GMT_end (argc, argv);
	
	exit (EXIT_SUCCESS);
}	
