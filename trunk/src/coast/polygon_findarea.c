/*
 *	$Id: polygon_findarea.c,v 1.2 2009-06-11 05:42:09 guru Exp $
 * Based on polygon_findlevel but limited to just compute polygon areas.
 */
#include "wvs.h"

int main (int argc, char **argv) {
	int k, ID, sign, n_alloc = 0;
	double size;
	double *flon = NULL, *flat = NULL;
	FILE *fp;
	struct LONGPAIR p;
	struct GMT3_POLY h;
	
	if (argc == 1) {
		fprintf (stderr, "usage: polygon_findarea final_dbase.b [ID]\n");
		fprintf (stderr, "Computes all areas unless a specific ID is given\n");
		fprintf (stderr, "Will write areas to stdout.\n");
		exit (-1);
	}

	ID = (argc == 3) ? atoi (argv[2]) : -1;
	
	fprintf (stderr, "Read headers\n");
	fp = fopen (argv[1], "r");
	
	/* Reset ids as we go along */
	
	area_init ();

	while (pol_readheader (&h, fp) == 1) {
		if (ID >= 0 && h.id != ID) {	/* Skip this polygon */
			fseek (fp, h.n * sizeof(struct LONGPAIR), 1);
			continue;
		}
		if (h.n > n_alloc) {
			n_alloc = h.n;
			flon = (double *) GMT_memory ((void *)flon, n_alloc, sizeof(double), "polygon_findarea");
			flat = (double *) GMT_memory ((void *)flat, n_alloc, sizeof(double), "polygon_findarea");
		}
		for (k = 0; k < h.n; k++) {
			if (pol_fread (&p, 1, fp) != 1) {
				fprintf(stderr,"polygon_findarea:  ERROR  reading file.\n");
				exit(-1);
			}
			if ((h.greenwich & 1) && p.x > h.datelon) p.x -= M360;
			flon[k] = p.x * 1.0e-6;
			flat[k] = p.y * 1.0e-6;
		}
		size = 1.0e-6 * area_size (flon, flat, h.n, &sign); /* in km^2 */
		printf ("%d\t%.12g\n", ID, size * sign);
	}
	
	fclose (fp);
	
	free ((void *)flon);
	free ((void *)flat);
	
	exit (0);
}	
