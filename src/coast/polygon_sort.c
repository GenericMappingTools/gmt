/*
 *	$Id: polygon_sort.c,v 1.1 2004-09-05 04:00:51 pwessel Exp $
 */
/* polygon_sorts writes out final data in decreasing order (# points)
 *
 */

#include "wvs.h"

struct GMT3_POLY hin;

struct BLOB {
	struct GMT3_POLY h;
	int pos, f;
} hh[200000];

struct LONGPAIR p[N_LONGEST];

main (argc, argv)
int	argc;
char **argv;
{
	FILE	*fp[2], *fp_out;
	int	id, n_id, n_1, pos, comp_blobs();

	if (argc != 4) {
		fprintf(stderr,"usage:  polygon_sort wvs_polygons.b cia_polygons.b polygons.b\n");
		exit(-1);
	}

	fp[0] = fopen(argv[1], "r");
	fp[1] = fopen(argv[2], "r");
	fp_out = fopen(argv[3], "w");
	
	id = pos = 0;
	while (pol_readheader (&hin, fp[0]) == 1) {
		
		hh[id].h = hin;
		hh[id].pos = pos;
		hh[id].f = 0;
		if (fseek (fp[0], hin.n * sizeof (struct LONGPAIR), SEEK_CUR)) {
			fprintf (stderr, "polygon_sort: Failed seeking ahead\n");
			exit (-1);
		}
		
		pos += (hin.n * sizeof (struct LONGPAIR) + sizeof (struct GMT3_POLY));
		if (hin.n > 1) id++;
	}
	
	n_1 = id;
	fprintf (stderr, "polygon_sort: Got %d polygons from file %s\n", n_1, argv[1]);

	pos = 0;
	while (pol_readheader (&hin, fp[1]) == 1) {
		
		hh[id].h = hin;
		hh[id].pos = pos;
		hh[id].f = 1;
		if (fseek (fp[1], hin.n * sizeof (struct LONGPAIR), SEEK_CUR)) {
			fprintf (stderr, "polygon_sort: Failed seeking ahead\n");
			exit (-1);
		}
		
		pos += (hin.n * sizeof (struct LONGPAIR) + sizeof (struct GMT3_POLY));
		id++;
	}
	
	n_id = id;

	fprintf (stderr, "polygon_sort: Got %d polygons from file %s\n", n_id - n_1, argv[2]);

	qsort ((char *)hh, n_id, sizeof (struct BLOB), comp_blobs);
	
	for (id = 0; id < n_id; id++) {
	
		if (fseek (fp[hh[id].f], hh[id].pos, SEEK_SET)) {
			fprintf (stderr, "polygon_sort: Failed seeking ahead\n");
			exit (-1);
		}
	
		if (pol_readheader (&hin, fp[hh[id].f]) != 1) {
			fprintf (stderr, "polygon_sort: Failed reading header\n");
			exit (-1);
		}

		if (pol_fread (p, hin.n, fp[hh[id].f]) != hin.n) {
			fprintf(stderr,"polygon_sort:  ERROR  reading file.\n");
			exit(-1);
		}
		
		hin.id = id;
		/* hin.source = 0;
		hin.level = -1; */
		
		if (pol_writeheader (&hin, fp_out) != 1) {
			fprintf (stderr, "polygon_sort: Failed writing header\n");
			exit(-1);
		}
		
		if (pol_fwrite (p, hin.n, fp_out) != hin.n) {
			fprintf(stderr,"polygon_sort:  ERROR  writing file.\n");
			exit(-1);
		}
	}
	
	fclose(fp_out);
	fclose(fp[0]);
	fclose(fp[1]);

	exit(0);
}

int comp_blobs (a, b)
struct BLOB *a, *b; {
	if (a->h.n > b->h.n) return (-1);
	if (a->h.n < b->h.n) return (1);
	return (0);
}
