/*
 *	$Id: polygon_bincount.c,v 1.4 2011-04-12 13:06:43 remko Exp $
 */
/* 
 *	polygon_bincount calculates # points pr bin
 */

#include "wvs.h"

int main (int argc, char **argv)
{
	FILE	*fp_in;
	int	k, i, j = 0, iw, nbins, *bin, *tmp, bwidth;
	double w;
	struct	LONGPAIR p;
	struct GMT3_POLY h;
        
	if (argc != 3) {
		fprintf(stderr,"usage: polygon_bincount polygons.b binsize\n");
		exit(-1);
	}

	fp_in = fopen(argv[1], "r");
	w = atof (argv[2]);
	bwidth = rint (360.0/w);
	nbins = bwidth * rint (180.0/w);
	iw = rint (w * 1.0e6);
	bin = (int*) GMT_memory (CNULL, nbins, sizeof (int), "polygon_bincount");
	tmp = (int*) GMT_memory (CNULL, nbins, sizeof (int), "polygon_bincount");
	
	while (pol_readheader (&h, fp_in) == 1) {
		memset ((char *)tmp, 0, nbins * sizeof (int));
		for (k = 0; k < h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf(stderr,"polygon_bincount: Error reading file.\n");
				exit(-1);
			}
			if (p.x == M360) p.x = 0;
			i = p.x / iw;
			j = (p.y + M90) / iw;
			tmp[j*bwidth+i]++;
		}
		for (i = 0; i < nbins; i++) {
			if (tmp[i] == 0) continue;
			if (tmp[i] > bin[i]) bin[i] = tmp[i];
		}
	}
		
	fclose(fp_in);

	for (i = k = 0; i < nbins; i++) {
		if (bin[i] == 0) continue;
		printf ("%d\t%d\n", i, bin[i]);
		if (bin[i] > k) k = bin[i], j = i;
	}
	
	fprintf (stderr, "Bin %d has the longest segment: %d points\n", j, k);
	
	free ((char *)bin);
	free ((char *)tmp);
	exit (0);
}
