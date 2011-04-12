/*
 *	$Id: polygon_compare.c,v 1.4 2011-04-12 13:06:43 remko Exp $
 */
/* 
 *
 */

#include "wvs.h"

#define NOISE GMT_CONV_LIMIT

int main (int argc, char **argv)
{
	FILE	*fp_a, *fp_b;
	int	i, nh_bad = 0, nd_bad = 0, n_id = 0;
	double f;
 	struct GMT3_POLY ha, hb;
	struct LONGPAIR *pa, *pb;

      
	if (argc != 3) {
		fprintf(stderr,"usage: polygon_compare one.b two.b\n");
		exit (EXIT_FAILURE);
	}

	pa = (struct LONGPAIR *)GMT_memory (VNULL, N_LONGEST, sizeof (struct LONGPAIR), "polygon_compare");
	pb = (struct LONGPAIR *)GMT_memory (VNULL, N_LONGEST, sizeof (struct LONGPAIR), "polygon_compare");
	
	fp_a = fopen (argv[1], "rb");
	fp_b = fopen (argv[2], "rb");
	
	while (pol_readheader (&ha, fp_a) == 1) {
		if (pol_readheader (&hb, fp_b) != 1) {
			fprintf (stderr, "polygon_compare: Header read error on file 2\n");
			exit (EXIT_FAILURE);
		}
		if (! (ha.id == hb.id)) {
			fprintf (stderr, "polygon_compare: headers ID for polygon %d differ\n", ha.id);
			nh_bad++;
		}
		if (! (ha.n == hb.n)) {
			fprintf (stderr, "polygon_compare: headers N for polygon %d differ\n", ha.id);
			nh_bad++;
		}
		if (! ((ha.greenwich & 1) == (hb.greenwich & 1))) {
			fprintf (stderr, "polygon_compare: headers G for polygon %d differ\n", ha.id);
			nh_bad++;
		}
		if (! (ha.level == hb.level)) {
			fprintf (stderr, "polygon_compare: headers L for polygon %d differ\n", ha.id);
			nh_bad++;
		}
		if (! (ha.datelon == hb.datelon)) {
			fprintf (stderr, "polygon_compare: headers E for polygon %d differ\n", ha.id);
			nh_bad++;
		}
		if (! (ha.source == hb.source)) {
			fprintf (stderr, "polygon_compare: headers S for polygon %d differ\n", ha.id);
			nh_bad++;
		}
		if (! (ha.parent == hb.parent)) {
			fprintf (stderr, "polygon_compare: headers P for polygon %d differ\n", ha.id);
			nh_bad++;
		}
		if (! (ha.ancestor == hb.ancestor)) {
			fprintf (stderr, "polygon_compare: headers F for polygon %d differ\n", ha.id);
			nh_bad++;
		}
		if (fabs (ha.west - hb.west) > NOISE) {
			fprintf (stderr, "polygon_compare: headers R:west for polygon %d differ\n", ha.id);
			nh_bad++;
		}
		if (fabs (ha.east - hb.east) > NOISE) {
			fprintf (stderr, "polygon_compare: headers R:east for polygon %d differ\n", ha.id);
			nh_bad++;
		}
		if (fabs (ha.south - hb.south) > NOISE) {
			fprintf (stderr, "polygon_compare: headers R:south for polygon %d differ\n", ha.id);
			nh_bad++;
		}
		if (fabs (ha.north - hb.north) > NOISE) {
			fprintf (stderr, "polygon_compare: headers R:north for polygon %d differ\n", ha.id);
			nh_bad++;
		}
		if ((f = fabs ((ha.area - hb.area)/ha.area)) > NOISE) {
			fprintf (stderr, "polygon_compare: headers A for polygon %d differ\n", ha.id);
			nh_bad++;
		}
		if (ha.area == 0.0) {
			fprintf (stderr, "polygon_compare: headers A for polygon %d is zero\n", ha.id);
			nh_bad++;
		}
		if ((f = fabs ((ha.area_res - hb.area_res)/ha.area_res)) > NOISE) {
			fprintf (stderr, "polygon_compare: headers B for polygon %d differ\n", ha.id);
			nh_bad++;
		}
		if (ha.area_res == 0.0) {
			fprintf (stderr, "polygon_compare: headers B for polygon %d is zero\n", ha.id);
			nh_bad++;
		}
		if (pol_fread (pa, ha.n, fp_a) != ha.n) {
			fprintf (stderr, "polygon_compare: Data read error on file 1!\n");
			exit (EXIT_FAILURE);
		}
		if (pol_fread (pb, hb.n, fp_b) != hb.n) {
			fprintf (stderr, "polygon_compare: Data read error on file 2!\n");
			exit (EXIT_FAILURE);
		}
		
		for (i = 0; i < ha.n; i++) {
			if (!(abs (pa[i].x - pb[i].x)%M360 == 0 && pa[i].y == pb[i].y)) {
				fprintf (stderr, "polygon_compare: data recs differ for polygon %d, rec %d\n", ha.id, i);
				nd_bad++;
			}
		}
		n_id++;
	}
	fclose (fp_a);
	fclose (fp_b);
	
	GMT_free ((void *)pa);
	GMT_free ((void *)pb);

	fprintf (stderr, "polygon_compare: Scanned: %d Headers differ: %d Points differ: %d\n", n_id, nh_bad, nd_bad);
	
	exit (EXIT_SUCCESS);
}
