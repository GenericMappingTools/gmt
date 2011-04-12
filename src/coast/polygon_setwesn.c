/*
 *	$Id: polygon_setwesn.c,v 1.4 2011-04-12 13:06:43 remko Exp $
 */
/* polygon_setwesn updates the wesn info
 *
 */

#include "wvs.h"

struct LONGPAIR p[N_LONGEST];

int main (int argc, char **argv)
{
	FILE	*fp;
	int	i, ix, ixmin, ixmax, iymin, iymax;
	struct GMT3_POLY h;

	if (argc != 2) {
		fprintf(stderr,"usage: polygon_setwesn wvs_polygons.b > new.b\n");
		exit(-1);
	}

	fp = fopen(argv[1], "r");
	
	while (pol_readheader (&h, fp) == 1) {
		ixmin = iymin = M360;
		ixmax = iymax = -M360;
		if (pol_fread (p, h.n, fp) != h.n) {
			fprintf(stderr,"polygon_setwesn: Error reading file.\n");
			exit(-1);
		}
		for (i = 0; i < h.n; i++) {
			ix = p[i].x;
			if ((h.greenwich & 1) && ix > h.datelon) ix -= M360;
			if (ix < ixmin) ixmin = ix;
			if (ix > ixmax) ixmax = ix;
			if (p[i].y < iymin) iymin = p[i].y;
			if (p[i].y > iymax) iymax = p[i].y;
		}
		if (abs (ixmax - ixmin) == M360) {	/* Antarctica */
			iymin = -M90;
			fprintf (stderr, "polygon_setwesn: Antarctica south set to -90\n");
		}
		h.west = ixmin * 1e-6;
		h.east = ixmax * 1e-6;
		h.south = iymin * 1e-6;
		h.north = iymax * 1e-6;
		if (pol_writeheader (&h, stdout) != 1) {
			fprintf(stderr,"polygon_setwesn: Error writing file.\n");
			exit(-1);
		}
		if (pol_fwrite (p, h.n, stdout) != h.n) {
			fprintf(stderr,"polygon_setwesn: Error writing file.\n");
			exit(-1);
		}
	}
	
	fclose(fp);

	exit(0);
}

