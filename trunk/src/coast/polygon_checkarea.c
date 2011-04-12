/*
 *	$Id: polygon_checkarea.c,v 1.4 2011-04-12 13:06:43 remko Exp $
 */
/* 
 *
 */

#include "wvs.h"

#define R 6378137.0

struct CHECK {
	double xm, ym;
	double west, east, south, north, area;
	int n, level;
} P[N_POLY];

int main (int argc, char **argv)
{
	FILE	*fp_in;
	int	n_id, i, j, k, r, start_id = 0, dup, other, d, ndup = 0, verbose = FALSE;
	double	x, y, ratio;
	struct	LONGPAIR p;
 	struct GMT3_POLY h3;
       
	if (argc < 3 || argc > 4) {
		fprintf(stderr,"usage: polygon_checkarea final_polygons.b start_id [-v] > remove.lis\n");
		exit(-1);
	}

	fp_in = fopen(argv[1], "r");
	start_id = atoi (argv[2]);
	
	verbose = (argc == 4);
		
	n_id = 0;
	while (pol_readheader (&h3, fp_in) == 1) {
		P[n_id].west = h3.west;
		P[n_id].east = h3.east;
		P[n_id].south = h3.south;
		P[n_id].north = h3.north;
		P[n_id].area = h3.area;
		P[n_id].n = h3.n;
		P[n_id].level = h3.level;
		x = y = 0.0;
		for (k = 0; k < h3.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf(stderr,"polygon_checkarea: Error reading file.\n");
				exit(-1);
			}
			if ((h3.greenwich & 1) && p.x > h3.datelon) p.x -= M360;
			x += p.x;
			y += p.y;
		}
		x *= (1.0e-6 / h3.n);
		y *= (1.0e-6 / h3.n);
		P[n_id].xm = x * D2R;
		P[n_id].ym = y * D2R;
		n_id++;
	}
	
	/* Start chec loop loop */
	
	fprintf(stderr,"polygon_checkarea: %d polygons, start at pol # %d\n", n_id, start_id);

	for (i = start_id; i < n_id; i++) {
	
		if (!(i%100)) fprintf (stderr, "%6d\t%d\r", i, ndup);	
				
		for (j = i + 1; j < n_id; j++) {
		
			if (P[i].north < P[j].south || P[i].south > P[j].north) continue;
			if (P[i].east < P[j].west || P[i].west > P[j].east) continue;
			
			/* Have some area in common */
			
			ratio = (P[j].area < P[i].area) ? (P[j].area / P[i].area) : (P[i].area / P[j].area);
			
			if (ratio < 0.5) continue;
			
			d = (int) rint (hypot ((P[i].xm - P[j].xm) * cos (P[i].ym), P[i].ym - P[j].ym) * R);
			
			r = (int) rint (1000.0 * sqrt (P[j].area / M_PI));
			
			if (d > r) continue;
			
			ndup++;
			
			if (P[j].level >= P[i].level) {
				dup = j;
				other = i;
			}
			else {
				dup = i;
				other = j;
			}
			
			if (ratio == 1.0 && P[i].n == P[j].n && d == 0)
				printf("%d exact duplicate of %d\n", dup, other);
			else
				printf("%d probable duplicate of %d [ %g %d %d %d ]\n", dup, other, ratio, P[i].n, P[j].n, d);
		}
	}
		
	fclose(fp_in);

	exit (0);
}
