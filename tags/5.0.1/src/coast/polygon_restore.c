/*
 *	$Id$
 */
/* 
 *
 */

#include "wvs.h"

struct	LONGPAIR p[N_LONGEST];
struct GMT3_POLY h;

int main (int argc, char **argv)
{
	FILE	*fp_out, *fp_new, *fp;
	int	i, j, k, n_pol_out = 0, level, reverse = 0;
	int	n_alloc, n_pt_out = 0, sign;
	double x, y;
	double *flon = NULL, *flat = NULL;
	char file[80], line[GMT_BUFSIZ];
        
	if (argc == 1) {
		fprintf(stderr,"usage: polygon_restore new.lis final_polygons.b level\n");
		exit(-1);
	}

	fp_new = fopen (argv[1], "r");
		
	fp_out = fopen(argv[2], "w");
	level = atoi (argv[3]);
	if (level < 1 || level > 4) {
		fprintf(stderr,"polygon_restore: Level not in 1-4 range\n");
		exit(-1);
	}
	
	area_init ();
	n_alloc = GMT_CHUNK;
	flon = (double *) GMT_memory (flon, n_alloc, sizeof(double), "polygon_restore");
	flat = (double *) GMT_memory (flat, n_alloc, sizeof(double), "polygon_restore");

	while (fgets (line, 80, fp_new)) {
		sscanf (line, "%d", &i);
		sprintf (file, "pol/polygon.%d", i);
		fprintf (stderr, "Restoring polygon %d\n", i);
		fp = fopen (file, "r");
		k = 0;
		h.west = h.south = 1.0e100;
		h.east = h.north = -1.0e100;
		while (fgets (line, GMT_BUFSIZ, fp)) {
			sscanf (line, "%lf %lf", &x, &y);
			p[k].x = (int) rint (x * 1.0e6);
			p[k].y = (int) rint (y * 1.0e6);
			if (x < h.west) h.west = x;
			if (x > h.east) h.east = x;
			if (y < h.south) h.south = y;
			if (y > h.north) h.north = y;
			flon[k] = x;
			flat[k] = y;
			k++;
			if (k == n_alloc) {
				n_alloc += GMT_CHUNK;
				flon = (double *) GMT_memory (flon, n_alloc, sizeof(double), "polygon_restore");
				flat = (double *) GMT_memory (flat, n_alloc, sizeof(double), "polygon_restore");
			}
		}
		fclose (fp);
		h.n = k;
		h.id = i;
		h.level = level;
		h.area = 1.0e-6 * area_size (flon, flat, h.n, &sign); /* in km^2 */
		if ( (h.level%2) && sign == -1)		/* Land and negative area -> must reverse order */
			reverse = 1;
		else if ( !(h.level%2) && sign == +1)	/* Water and positive area -> must reverse order */
			reverse = 1;
		else
			reverse = 0;
			
		/* Write out */
		
		if (pol_writeheader (&h, fp_out) != 1) {
			fprintf (stderr, "polygon_restore: Write error!\n");
			exit (-1);
		}
		
		if (reverse) {
			fprintf (stderr, "polygon_restore: Must reverse polygon %d\n", h.id);
			for (j = h.n - 1; j >= 0; j--) {
				if (pol_fwrite (&p[j], 1, fp_out) != 1) {
					fprintf (stderr, "polygon_restore: Write error!\n");
					exit (-1);
				}
			}
		}
		else {
			if (pol_fwrite (p, h.n, fp_out) != h.n) {
				fprintf (stderr, "polygon_restore: Write error!\n");
				exit (-1);
			}
		}
		
		n_pol_out++;
		n_pt_out += h.n;
	}
	fclose (fp_new);
	
	fclose(fp_out);

	fprintf (stderr, "polygon_restore: Polygons written: %d	Points written: %d\n", n_pol_out, n_pt_out);
	
	exit (0);
}
