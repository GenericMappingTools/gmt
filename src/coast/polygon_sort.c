/*
 *	$Id$
 */
/* polygon_sorts writes out final data in decreasing order (# points)
 *
 */

#include "wvs.h"

struct GMT3_POLY hin;

struct BLOB {
	struct GMT3_POLY h;
	int pos;
} hh[200000];

struct LONGPAIR p[N_LONGEST];

int main (int argc, char **argv)
{
	FILE	*fp, *fp_out;
	int	id, n_id, do_n, do_a, pos, k, sign, comp_blobs_a(), comp_blobs_n();
	double *flon = NULL, *flat = NULL;
	struct LONGPAIR pp;

	if (argc < 3) {
		fprintf(stderr,"usage: polygon_sort old.b new.b [-a|A|n]\n");
		exit(-1);
	}

	fp = fopen(argv[1], "r");
	fp_out = fopen(argv[2], "w");
	do_n = (argc >= 4 && !strcmp (argv[3], "-n"));
	do_a = (argc >= 4 && !strcmp (argv[3], "-A"));
	if (do_a) {	/* Must recalculate areas */
		fprintf (stderr, "polygon_sort: Will recalculate polygon areas\n");
		area_init ();
		flon = (double *) GMT_memory (CNULL, N_LONGEST, sizeof(double), "polygon_findlevel");
		flat = (double *) GMT_memory (CNULL, N_LONGEST, sizeof(double), "polygon_findlevel");
	}
	if (do_n)
		fprintf (stderr, "polygon_sort: Will sort based on number of verteces\n");
	else
		fprintf (stderr, "polygon_sort: Will sort based on polygon areas\n");
	id = pos = 0;
	while (pol_readheader (&hin, fp) == 1) {
		
		hh[id].h = hin;
		hh[id].pos = pos;
		
		for (k = 0; k < hin.n; k++) {
			if (pol_fread (&pp, 1, fp) != 1) {
				fprintf(stderr,"polygon_findlevel: Error reading file.\n");
				exit(-1);
			}
			if (do_a) {
				if ((hin.greenwich & 1) && pp.x > hin.datelon) pp.x -= M360;
				flon[k] = pp.x * 1.0e-6;
				flat[k] = pp.y * 1.0e-6;
			}
		}
		if (do_a) hh[id].h.area = 1.0e-6 * area_size (flon, flat, hin.n, &sign); /* in km^2 */
		
		if (hh[id].h.river & 1) hh[id].h.area = -fabs(hh[id].h.area);
		
		pos += (hin.n * sizeof (struct LONGPAIR) + sizeof (struct GMT3_POLY));
		id++;
	}
	n_id = id;

	fprintf (stderr, "polygon_sort: Got %d polygons from file %s\n", n_id, argv[1]);

	if (do_n)
		qsort ((char *)hh, n_id, sizeof (struct BLOB), comp_blobs_n);
	else
		qsort ((char *)hh, n_id, sizeof (struct BLOB), comp_blobs_a);
	
	for (id = 0; id < n_id; id++) {
	
		if (fseek (fp, hh[id].pos, SEEK_SET)) {
			fprintf (stderr, "polygon_sort: Failed seeking ahead\n");
			exit (-1);
		}
	
		if (pol_readheader (&hin, fp) != 1) {
			fprintf (stderr, "polygon_sort: Failed reading header\n");
			exit (-1);
		}

		if (pol_fread (p, hin.n, fp) != hin.n) {
			fprintf(stderr,"polygon_sort: Error reading file.\n");
			exit(-1);
		}
		
		hin.id = id;
		if (do_a || hh[id].h.area == 0.0) hin.area = fabs(hh[id].h.area);
		
		if (pol_writeheader (&hin, fp_out) != 1) {
			fprintf (stderr, "polygon_sort: Failed writing header\n");
			exit(-1);
		}
		
		if (pol_fwrite (p, hin.n, fp_out) != hin.n) {
			fprintf(stderr,"polygon_sort: Error writing file.\n");
			exit(-1);
		}
	}
	
	fclose(fp_out);
	fclose(fp);
	if (do_a) {
		GMT_free ((void *)flon);
		GMT_free ((void *)flat);
	}
	exit(0);
}

int comp_blobs_a (a, b)
struct BLOB *a, *b; {
	/* Sort on level, then on area, arranging riverlakes after lakes */
	if (a->h.level < b->h.level) return (-1);
	if (a->h.level > b->h.level) return (+1);
	if (a->h.area > 0.0 && b->h.area < 0.0) return (-1);
	if (a->h.area < 0.0 && b->h.area > 0.0) return (+1);
	if (fabs(a->h.area) > fabs(b->h.area)) return (-1);
	if (fabs(a->h.area) < fabs(b->h.area)) return (+1);
	return (0);
}


int comp_blobs_n (a, b)
struct BLOB *a, *b; {
	/* Sort on number of points */
	if (a->h.n > b->h.n) return (-1);
	if (a->h.n < b->h.n) return (1);
	return (0);
}
