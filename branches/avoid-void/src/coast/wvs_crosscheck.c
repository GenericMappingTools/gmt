/*
 *	$Id$
 */
/*  */
   
#include "wvs.h"

double x[N_LONGEST], y[N_LONGEST];
struct GMT3_POLY h;
struct GMT_XOVER c;
struct GMT_XSEGMENT *ylist;
struct LONGPAIR p[N_LONGEST];
int no[N_LONGEST];

int main (int argc, char **argv) {
	int id = 0, i, nn = 0, k, nx, j, cut, nx_tot = 0, n_bad = 0, max_n = 0, max_id = 0, end, A, B, n_found;
	int start, stop, report, n_fatal = 0, n_too_much = 0, special, update = FALSE;
	FILE *fp, *fp_out = NULL;
	
	if (argc < 2 || argc > 3) {
		fprintf (stderr, "usage: wvs_crosscheck coast.base [newbase]\n");
		exit (-1);
	}
	
	fp = fopen (argv[1], "r");	
	if (argc == 3) {
		fp_out = fopen (argv[2], "w");
		update = TRUE;
	}
	
	while (pol_readheader (&h, fp) == 1) {
		h.id = id;
		if (pol_fread (p, h.n, fp) != h.n) {
			fprintf(stderr,"wvs_crosscheck: Error reading file.\n");
			exit(-1);
		}
		
		for (i = 0; i < h.n; i++) no[i] = i + 1;
		
		if (fabs (h.west - h.east) == 360.0) {
			special = TRUE;
			h.west = 0.0;
			h.east = 360.0;
			h.greenwich = FALSE;
		}
		else
			special = FALSE;
			
		if (h.greenwich) for (i = 0; i < h.n; i++) if (p[i].x > h.datelon) p[i].x -= 360000000;
		
		if (special) p[0].x = 360000000;
		
		for (i = 0; i < h.n; i++) {
			x[i] = p[i].x * 1.0e-6;
			y[i] = p[i].y * 1.0e-6;
		}
		
		report = FALSE;
		
		GMT_init_track (y, h.n, &ylist);
		nx = n_found = GMT_crossover (x, y, NULL, ylist, h.n, x, y, NULL, ylist, h.n, TRUE, &c);
		
		for (i = end = 0; i < nx; i++) {
			A = irint (c.xnode[0][i]);
			B = irint (c.xnode[1][i]);
			if ((A == 0 && B == (h.n-1)) || (B == 0 && A == (h.n-1))) {
				/* Remove the crossover caused by the duplicate start/end points */
				end++;
			}
		}
		nx -= end;
		if (nx && special) {
			free (c.x);
			free (c.y);
			free (c.xnode[0]);
			free (c.xnode[1]);
			nx = n_found = 0;
		}
		if (!update && nx) {
			printf ("%s: Polygon %d has %d crossovers\n", argv[0], h.id, nx);
		}
		if (!update && n_found) {
			free (c.x);
			free (c.y);
			free (c.xnode[0]);
			free (c.xnode[1]);
			nx = 0;
		}
		if (nx) {	/* Must chop off the bad sections */
			nn++;
			nx_tot += nx;
			for (i = 0; i < nx; i++) {
				start = (int)ceil (c.xnode[0][i]);
				stop = (int) floor (c.xnode[1][i]);
				cut = abs (stop - start) + 1;
				if (cut > h.n/2) {
					cut = h.n - cut;
					i_swap (start, stop);
				}
				if (cut > 5) n_bad++;
				if (cut > max_n) {
					max_n = cut;
					max_id = id;
				}
				
				if (cut < 50) {
					/* Mark all the points between start and stop as bad (negative k) */
				
					k = start - 1;
					while (k != stop) {
						k++;
						if (k == h.n) k = 0;
						no[k] = -1;
					}
				}
				else
					n_too_much++;
			}
			for (k=0, j=0; k < h.n; k++) {
				p[j] = p[k];
				if (no[k] > 0) j++;
			}
			h.n = j;
			free (c.x);
			free (c.y);
			free (c.xnode[0]);
			free (c.xnode[1]);
			
			/* Make sure it worked ok */
			
			for (i = 0; i < h.n; i++) {
				x[i] = p[i].x * 1.0e-6;
				y[i] = p[i].y * 1.0e-6;
			}
		
			nx = GMT_crossover (x, y, NULL, ylist, h.n, x, y, NULL, ylist, h.n, TRUE, &c);
			
			if (nx) {	/* Shit... */
				printf ("\nPolygon # %d still has %d xovers\n", h.id, nx);
				for (i = 0; i < nx; i++) printf ("%g\t%g\n", c.x[i], c.y[i]);
				free (c.x);
				free (c.y);
				free (c.xnode[0]);
				free (c.xnode[1]);
				n_fatal++;
			}
			report = TRUE;
			
		}
		
		free (ylist);
		/* Write out new trimmed polygon */
		
		if (update && h.n > 2) {
			if (pol_writeheader (&h, fp_out) != 1) {
				fprintf (stderr, "polygon_crosscheck: Failed writing header\n");
				exit(-1);
			}
			
			if (h.greenwich) for (i = 0; i < h.n; i++) if (p[i].x < 0) p[i].x += 360000000;

			if (pol_fwrite (p, h.n, fp_out) != h.n) {
				fprintf(stderr,"polygon_crosscheck: Error writing file.\n");
				exit(-1);
			}
		}
				
		if (update && (report || h.id%1000 == 0)) fprintf (stderr, "wvs_crosscheck: processed # %d, %d has crossings\r", h.id, nn);
		id++;
		
	}
	fclose (fp);
	if (update) {
		fclose (fp_out);
	
		fprintf (stderr, "\n");
	
		fprintf (stderr, "wvs_crosscheck: processed %d polygons, %d polygons have total of %d crossings\n", id, nn, nx_tot);
		fprintf (stderr, "wvs_crosscheck: %d of these crossings were easy to fix, %d were hard\n", nx_tot - n_bad, n_bad);
		fprintf (stderr, "wvs_crosscheck: In worst case (id = %d), %d points would be chopped\n", max_id, max_n);
		if (n_fatal) fprintf (stderr, "wvs_crosscheck: %d polygons were unfixable\n", n_fatal);
		if (n_too_much) fprintf (stderr, "wvs_crosscheck: %d crossings would require > 50 points cut\n", n_too_much);
	}
	
	exit (0);
}

