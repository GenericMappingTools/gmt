/*
 *	$Id: polygon_stats.c,v 1.5 2011-04-12 13:06:42 remko Exp $
 */
/* 
 *	polygon_stats finds the average point spacing
 */

#include "wvs.h"

#define DEGREE_TO_KM	111.195

int main (int argc, char **argv)
{
	FILE	*fp_in;
	int	n = 0, k, nk[12], i_max = 0;
	struct	LONGPAIR p;
	struct GMT3_POLY h;
	double ds, sum = 0.0, sum2 = 0.0, x0, x1, y0, y1;
	double ds_min = 0.0, ds_max = 0.0, dx;
        
	if (argc == 1) {
		fprintf(stderr,"usage: polygon_stats polygons.b\n");
		exit(-1);
	}
	
	memset ((char *)nk, 0, 12 * sizeof (int));

	fp_in = fopen(argv[1], "r");
		
	while (pol_readheader (&h, fp_in) == 1) {
		if (h.level != 2) {
			fseek (fp_in, h.n * sizeof(struct LONGPAIR), 1);
			continue;
		}
		
		if (pol_fread (&p, 1, fp_in) != 1) {
			fprintf(stderr,"polygon_stats: Error reading file.\n");
			exit(-1);
		}
		if ((h.greenwich & 1) && p.x > h.datelon) p.x -= M360;
		x1 = 1.0e-6 * p.x;
		y1 = 1.0e-6 * p.y;
		for (k = 1; k < h.n; k++) {
			x0 = x1;
			y0 = y1;
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf(stderr,"polygon_stats: Error reading file.\n");
				exit(-1);
			}
			if ((h.greenwich & 1) && p.x > h.datelon) p.x -= M360;
			x1 = 1.0e-6 * p.x;
			y1 = 1.0e-6 * p.y;
			dx = fabs(x1 - x0);
			if (dx > 180.0) dx = 360.0 - dx;
			ds = DEGREE_TO_KM * hypot (dx * cosd (0.5 * (y0 + y1)), y1 - y0);
			if (ds == 0.0) continue;
			sum += ds;
			sum2 += ds * ds;
			if (ds > ds_max) {
				ds_max = ds;
				i_max = h.id;
			}
			if (ds < ds_min) ds_min = ds;
		}
		n += (h.n - 1);
		k = floor (log10 (h.area)) + 4;
		nk[k]++;
	}
		
	fclose(fp_in);

	sum2 = sqrt ((n * sum2 - sum * sum) / (n * (n - 1.0)));
	sum = sum / n;
	fprintf (stderr, "Mean point separation was %10.3f km +- %10.3f km\n", sum, sum2);
	fprintf (stderr, "Min/max point separation was %10.3f km and %10.3f km (%d)\n", ds_min, ds_max, i_max);
	for (k = 0; k < 12; k++) printf ("%d	%d\n", k - 4, nk[k]);

	exit (0);
}
