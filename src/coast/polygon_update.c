/*
 *	$Id$
 */
/* 
 *
 */

#include "wvs.h"

int bad[11001], fix[10000];

struct	LONGPAIR p[N_LONGEST];
struct GMT3_POLY h;

int main (int argc, char **argv)
{
	FILE	*fp_in, *fp_out, *fp_bad, *fp_fix, *fp;
	int	i, j, found, k, n_id, nfix, nbad, n_pol_in = 0, n_pol_out = 0, reverse = 0;
	int	n_alloc = 0, n_pt_in = 0, n_pt_out = 0, sign, full = 0, get_area = 0;
	double x, y, size;
	double *flon = NULL, *flat = NULL;
	char file[80], line[512];
        
	if (argc == 1) {
		fprintf(stderr,"usage: polygon_update final_polygons.b bad.lis fix.lis final_x_polygons.b [-n]\n");
		fprintf(stderr,"	-n Do not recalculate area and check for handedness\n");
		fprintf(stderr,"	   By default, for the full resolution we will update region and area\n");
		fprintf(stderr,"	   We will reverse the order of the polygon if level and handedness are in conflict\n");
		fprintf(stderr,"	   The level is assumed to be correct.\n");
		exit(-1);
	}

	fp_bad = fopen (argv[2], "r");
	nbad = 0;
	while (fgets (line, 80, fp_bad)) {
		sscanf (line, "%d", &bad[nbad]);
		nbad++;
	}
	fclose (fp_bad);
	
	fp_fix = fopen (argv[3], "r");
	nfix = 0;
	while (fgets (line, 80, fp_fix)) {
		sscanf (line, "%d", &fix[nfix]);
		nfix++;
	}
	fclose (fp_fix);
	
	fp_in = fopen(argv[1], "r");
	fp_out = fopen(argv[4], "w");
	
	if (get_area) {
		area_init ();
		n_alloc = GMT_CHUNK;
		flon = (double *) GMT_memory (flon, n_alloc, sizeof(double), "polygon_findarea");
		flat = (double *) GMT_memory (flat, n_alloc, sizeof(double), "polygon_findarea");
	}
	n_id = 0;	
	while (pol_readheader (&h, fp_in) == 1) {
		
		/* h.id = n_id++; */
		n_pol_in++;
		n_pt_in += h.n;
		if (h.id == 0 && h.n > 1400000) full = TRUE;
		
		for (i = found = 0; i < nbad && !found; i++) if (bad[i] == h.id) found = TRUE;
		if (found) {
			fprintf (stderr, "Remove polygon %d\n", h.id);
			nbad--;
			bad[i-1] = bad[nbad];
			fseek (fp_in, h.n * sizeof (struct LONGPAIR), SEEK_CUR);
			continue;	/* Delete this from output file */
		}
		
		for (i = found = 0; i < nfix && !found; i++) if (fix[i] == h.id) found = TRUE;
		if (found) {	/* Replace this polygon */
			fseek (fp_in, h.n * sizeof (struct LONGPAIR), SEEK_CUR);
			nfix--;
			fix[i-1] = fix[nfix];
			sprintf (file, "pol/polygon.%d", h.id);
			fprintf (stderr, "Replacing polygon %d\n", h.id);
			fp = fopen (file, "r");
			k = 0;
			h.west = h.south = 1.0e100;
			h.east = h.north = -1.0e100;
			while (fgets (line, 512, fp)) {
				sscanf (line, "%lf %lf", &x, &y);
				p[k].x = (int) rint (x * 1.0e6);
				p[k].y = (int) rint (y * 1.0e6);
				if (x < h.west) h.west = x;
				if (x > h.east) h.east = x;
				if (y < h.south) h.south = y;
				if (y > h.north) h.north = y;
				if (full && get_area) {
					flon[k] = x;
					flat[k] = y;
					k++;
					if (k == n_alloc) {
						n_alloc += GMT_CHUNK;
						flon = (double *) GMT_memory (flon, n_alloc, sizeof(double), "polygon_findarea");
						flat = (double *) GMT_memory (flat, n_alloc, sizeof(double), "polygon_findarea");
					}
				}
				else
					k++;
			}
			fclose (fp);
			h.n = k;
			if (full && get_area) {
				size = 1.0e-6 * area_size (flon, flat, h.n, &sign); /* in km^2 */
				if ( (h.level%2) && sign == -1)		/* Land and negative area -> must reverse order */
					reverse = 1;
				else if ( !(h.level%2) && sign == +1)	/* Water and positive area -> must reverse order */
					reverse = 1;
				else
					reverse = 0;
				if (!doubleAlmostEqualZero (size, h.area)) {
					fprintf (stderr, "polygon_update: Area revised for polygon %d [From %g to %g]\n", h.id, h.area, size);
					h.area = size;
				}
			}
		}
		else if (pol_fread (p, h.n, fp_in) != h.n) {
			fprintf (stderr, "polygon_update: read error!\n");
			exit (-1);
		}

		/* Write out */
		
		if (pol_writeheader (&h, fp_out) != 1) {
			fprintf (stderr, "polygon_update: Write error!\n");
			exit (-1);
		}
		
		if (reverse) {
			fprintf (stderr, "polygon_update: Must reverse polygon %d\n", h.id);
			for (j = h.n - 1; j >= 0; j--) {
				if (pol_fwrite (&p[j], 1, fp_out) != 1) {
					fprintf (stderr, "polygon_update: Write error!\n");
					exit (-1);
				}
			}
		}
		else {
			if (pol_fwrite (p, h.n, fp_out) != h.n) {
				fprintf (stderr, "polygon_update: Write error!\n");
				exit (-1);
			}
		}
		
		n_pol_out++;
		n_pt_out += h.n;
	}
	
	fclose(fp_in);
	fclose(fp_out);

	fprintf (stderr, "polygon_update: Polygons read   : %d	Points read   : %d\n", n_pol_in, n_pt_in);
	fprintf (stderr, "polygon_update: Polygons written: %d	Points written: %d\n", n_pol_out, n_pt_out);
	
	exit (0);
}
