/*
 *	$Id$
 * Based on polygon_findlevel but limited to just compute polygon areas.
 */
#include "wvs.h"

int main (int argc, char **argv) {
	int k, sign, n_alloc = 0;
	double size, f, cut = 0.0;
	double *flon = NULL, *flat = NULL;
	FILE *fp;
	struct LONGPAIR p;
	struct GMT3_POLY h;
	
	if (argc == 1) {
		fprintf (stderr, "usage: polygon_findarea final_dbase.b cut\n");
		fprintf (stderr, "Reports polygons whose area is less that cut %% of full res area\n");
		exit (-1);
	}

	cut = atof (argv[2]);
	
	fprintf (stderr, "Read headers\n");
	fp = fopen (argv[1], "r");
	
	/* Reset ids as we go along */
	
	area_init ();

	while (pol_readheader (&h, fp) == 1) {
		if (h.n > n_alloc) {
			n_alloc = h.n;
			flon = (double *) GMT_memory ((void *)flon, n_alloc, sizeof(double), "polygon_findarea");
			flat = (double *) GMT_memory ((void *)flat, n_alloc, sizeof(double), "polygon_findarea");
		}
		if (h.id == 315) {
			k = 9;
		}
		for (k = 0; k < h.n; k++) {
			if (pol_fread (&p, 1, fp) != 1) {
				fprintf(stderr,"polygon_findarea: Error reading file.\n");
				exit(-1);
			}
			if ((h.greenwich & 1) && p.x > h.datelon) p.x -= M360;
			flon[k] = p.x * 1.0e-6;
			flat[k] = p.y * 1.0e-6;
		}
		size = 1.0e-6 * area_size (flon, flat, h.n, &sign); /* in km^2 */
		f = 100.0 * (size / h.area);
		if (f < cut) printf ("%d\t%.12g vs %.12g\n", h.id, size, h.area);
	}
	
	fclose (fp);
	
	free ((void *)flon);
	free ((void *)flat);
	
	exit (0);
}	
