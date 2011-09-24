/*
 *	$Id$
 * Updates polygon areas for the resolution.
 */
#include "wvs.h"

int main (int argc, char **argv) {
	int k, x, sign, n_alloc = 0;
	double *flon = NULL, *flat = NULL;
	FILE *fp, *fp_out;
	struct LONGPAIR *p = NULL;
	struct GMT3_POLY h;
	
	if (argc != 3) {
		fprintf (stderr, "usage: polygon_setarea final_dbase.b new.b\n");
		fprintf (stderr, "Sets polygon area_res for all polygons\n");
		exit (-1);
	}

	fp = fopen (argv[1], "r");
	fp_out = fopen (argv[2], "w");
		
	area_init ();

	fprintf (stderr, "Read headers\n");
	while (pol_readheader (&h, fp) == 1) {
		if (h.n > n_alloc) {
			n_alloc = h.n;
			flon = (double *) GMT_memory ((void *)flon, n_alloc, sizeof(double), "polygon_setarea");
			flat = (double *) GMT_memory ((void *)flat, n_alloc, sizeof(double), "polygon_setarea");
			p = (struct LONGPAIR *) GMT_memory ((void *)p, n_alloc, sizeof(struct LONGPAIR), "polygon_setarea");
		}
		if (pol_fread (p, h.n, fp) != h.n) {
			fprintf(stderr,"polygon_setarea: Error reading file.\n");
			exit(-1);
		}
		for (k = 0; k < h.n; k++) {
			x = ((h.greenwich & 1) && p[k].x > h.datelon) ? p[k].x - M360 : p[k].x;
			flon[k] = x * 1.0e-6;
			flat[k] = p[k].y * 1.0e-6;
		}
		h.area_res = 1.0e-6 * area_size (flon, flat, h.n, &sign); /* in km^2 */
		if(pol_writeheader (&h, fp_out) != 1) {
			fprintf (stderr, "polygon_setarea: Failed writing header\n");
			exit(-1);
		}
		if (pol_fwrite (p, h.n, fp_out) != h.n) {
			fprintf(stderr,"polygon_setarea: Error writing file.\n");
			exit(-1);
		}
	}
	
	fclose (fp);
	fclose (fp_out);
	
	free ((void *)flon);
	free ((void *)flat);
	free ((void *)p);
	
	exit (0);
}	
