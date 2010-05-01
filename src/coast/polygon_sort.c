/*
 *	$Id: polygon_sort.c,v 1.6 2010-05-01 01:42:21 guru Exp $
 *
 * polygon_sorts writes out final data in decreasing order.  The order
 * is either by decreasing number of points (-n) or area (-A|a).  The
 * -A choice forces a recalculation of the areas whereas -a [the default]
 * uses the areas already present in the headers.
 * For area sorting we first sort on the level (1 to 4), then on area,
 * and arranging the riverlakes (level 2 but negative area) at the end of
 * the other lakes.
 *
 * Paul Wessel, 2010-04-30 Updated
 */

#include "wvs.h"

#define MAX_POL 200000

struct GMT3_POLY hin;

struct BLOB {
	struct GMT3_POLY h;
	int pos;
} hh[MAX_POL];

struct LONGPAIR p[N_LONGEST];

int main (int argc, char **argv)
{
	FILE *fp = NULL, *fp_out = NULL;
	int id, n_id, do_n, do_a, pos, k, sign, comp_blobs_a(), comp_blobs_n();
	double *flon = NULL, *flat = NULL;
	struct LONGPAIR pp;

	if (argc < 3) {
		fprintf (stderr,"usage:  polygon_sort old.b new.b [-a|A|n]\n");
		exit (-1);
	}

	fp = fopen (argv[1], "rb");
	fp_out = fopen (argv[2], "wb");
	do_n = (argc >= 4 && !strcmp (argv[3], "-n"));
	do_a = (argc >= 4 && !strcmp (argv[3], "-A"));
	if (do_a) {	/* Must recalculate areas */
		fprintf (stderr, "polygon_sort: Will recalculate polygon areas\n");
		area_init ();
		flon = (double *) GMT_memory (NULL, N_LONGEST, sizeof(double), "polygon_sort");
		flat = (double *) GMT_memory (NULL, N_LONGEST, sizeof(double), "polygon_sort");
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
				fprintf(stderr,"polygon_sort:  ERROR  reading file.\n");
				exit(-1);
			}
			if (do_a) {
				if ((hin.greenwich & 1) && pp.x > hin.datelon) pp.x -= M360;
				flon[k] = pp.x * 1.0e-6;
				flat[k] = pp.y * 1.0e-6;
			}
		}
		if (do_a) hh[id].h.area = 1.0e-6 * area_size (flon, flat, hin.n, &sign); /* in km^2 */
		
		if (hh[id].h.river) hh[id].h.area = -fabs(hh[id].h.area);	/* Make sure area for riverlakes is negative */
		
		pos += (hin.n * sizeof (struct LONGPAIR) + sizeof (struct GMT3_POLY));
		id++;
		if (id == MAX_POL) {
			fprintf(stderr,"polygon_sort:  Too many polygons, recompile after changing MAX_POL\n");
			exit(-1);
		}
	}
	n_id = id;

	fprintf (stderr, "polygon_sort: Got %d polygons from file %s\n", n_id, argv[1]);

	if (do_n)
		qsort ((char *)hh, n_id, sizeof (struct BLOB), comp_blobs_n);
	else
		qsort ((char *)hh, n_id, sizeof (struct BLOB), comp_blobs_a);
	
	fprintf (stderr, "polygon_sort: Write out sorted polygons to file %s\n", argv[2]);
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
			fprintf(stderr,"polygon_sort:  ERROR  reading file.\n");
			exit (-1);
		}
		
		hin.id = id;
		if (do_a || hh[id].h.area == 0.0) hin.area = fabs(hh[id].h.area);
		
		if (pol_writeheader (&hin, fp_out) != 1) {
			fprintf (stderr, "polygon_sort: Failed writing header\n");
			exit (-1);
		}
		
		if (pol_fwrite (p, hin.n, fp_out) != hin.n) {
			fprintf(stderr,"polygon_sort:  ERROR  writing file.\n");
			exit (-1);
		}
	}
	
	fclose (fp_out);
	fclose (fp);
	if (do_a) {
		GMT_free ((void *)flon);
		GMT_free ((void *)flat);
	}
	fprintf (stderr, "polygon_sort: Done\n");
	exit (0);
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
