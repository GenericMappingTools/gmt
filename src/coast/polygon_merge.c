/*
 *	$Id: polygon_merge.c,v 1.3 2011-04-12 13:06:42 remko Exp $
 */
/* polygon_merge 
 *
 */

#include "wvs.h"

struct GMT3_POLY hin;

struct BLOB {
	struct GMT3_POLY h;
	int pos, f;
} hh[200000];

struct LONGPAIR p[N_LONGEST];

int main (int argc, char **argv)
{
	FILE	*fp[2], *fp_out;
	int	i, id, n_id, n_1, n_2, pos, found;

	if (argc != 4) {
		fprintf(stderr,"usage: polygon_merge main.b updated.b.b final.b\n");
		exit(-1);
	}

	fp[0] = fopen(argv[1], "r");
	fp[1] = fopen(argv[2], "r");
	fp_out = fopen(argv[3], "w");
	
	id = pos = 0;
	while (pol_readheader (&hin, fp[0]) == 1) {
		
		hh[id].h = hin;
		hh[id].h.id = id;
		hh[id].pos = pos;
		hh[id].f = 0;
		if (fseek (fp[0], hin.n * sizeof (struct LONGPAIR), 1)) {
			fprintf (stderr, "polygon_merge: Failed seeking ahead\n");
			exit (-1);
		}
		
		pos += (hin.n * sizeof (struct LONGPAIR) + sizeof (struct GMT3_POLY));
		id++;
	}
	
	n_1 = id;
	fprintf (stderr, "polygon_merge: Got %d polygons from file %s\n", n_1, argv[1]);

	pos = n_2 = 0;
	while (pol_readheader (&hin, fp[1]) == 1) {
		
		hh[id].h = hin;
		hh[id].pos = pos;
		hh[id].f = 1;
		if (fseek (fp[1], hin.n * sizeof (struct LONGPAIR), 1)) {
			fprintf (stderr, "polygon_merge: Failed seeking ahead\n");
			exit (-1);
		}
		
		pos += (hin.n * sizeof (struct LONGPAIR) + sizeof (struct GMT3_POLY));
		id++;
		n_2++;
	}
	
	n_id = id;

	fprintf (stderr, "polygon_merge: Got %d polygons from file %s\n", n_id - n_1, argv[2]);

	for (id = 0; id < n_1; id++) {
	
		for (i = n_1, found = FALSE; !found && i < n_id; i++) if (hh[i].h.id == hh[id].h.id) found = TRUE;
		
		if (found) {	/* Use file 2 */
			i--;
			if (fseek (fp[1], hh[i].pos, 0)) {
				fprintf (stderr, "polygon_merge: Failed seeking ahead\n");
				exit (-1);
			}
	
			if (pol_readheader (&hin, fp[1]) != 1) {
				fprintf (stderr, "polygon_merge: Failed reading header\n");
				exit (-1);
			}

			if (pol_fread (p, hin.n, fp[1]) != hin.n) {
				fprintf(stderr,"wvs_final_dump: Error reading file.\n");
				exit(-1);
			}
		}
		else {
		
			if (fseek (fp[0], hh[id].pos, 0)) {
				fprintf (stderr, "polygon_merge: Failed seeking ahead\n");
				exit (-1);
			}
	
			if (pol_readheader (&hin, fp[0]) != 1) {
				fprintf (stderr, "polygon_merge: Failed reading header\n");
				exit (-1);
			}
			if (pol_fread (p, hin.n, fp[0]) != hin.n) {
				fprintf(stderr,"wvs_final_dump: Error reading file.\n");
				exit(-1);
			}
		}
			
		hin.id = hh[id].h.id;
			
		if (pol_writeheader (&hin, fp_out) != 1) {
			fprintf (stderr, "polygon_merge: Failed writing header\n");
			exit(-1);
		}
		
		if (pol_fwrite (p, hin.n, fp_out) != hin.n) {
			fprintf(stderr,"wvs_final_dump: Error writing file.\n");
			exit(-1);
		}
	}
	
	fclose(fp_out);
	fclose(fp[0]);
	fclose(fp[1]);

	exit(0);
}
