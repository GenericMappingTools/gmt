/*
 *	$Id: polygon_update.c,v 1.1 2004-09-05 04:00:51 pwessel Exp $
 */
/* 
 *
 */

#include "wvs.h"

int bad[11001], fix[10000];

struct	LONGPAIR p[N_LONGEST];
struct GMT3_POLY h;

main (argc, argv)
int	argc;
char **argv;
{
	FILE	*fp_in, *fp_out, *fp_bad, *fp_fix, *fp;
	int	i, found, k, n_id, nfix, nbad, n_pol_in = 0, n_pol_out = 0, n_pt_in = 0, n_pt_out = 0;
	double x, y;
	char file[80], line[512];
        
	if (argc == 1) {
		fprintf(stderr,"usage:  polygon_update final_polygons.b bad.lis fix.lis final_x_polygons.b\n");
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
			
	n_id = 0;	
	while (pol_readheader (&h, fp_in) == 1) {
		
		/* h.id = n_id++; */
		n_pol_in++;
		n_pt_in += h.n;
		
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
			sprintf (file, "pol/polygon.%d\0", h.id);
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
				k++;
			}
			fclose (fp);
			h.n = k;
		}
		else if (pol_fread (p, h.n, fp_in) != h.n) {
			fprintf (stderr, "polygon_update: read error!\n");
			exit (-1);
		}

		h.checked[0] = h.checked[1] = 1;
		
		/* Write out */
		
		if (pol_writeheader (&h, fp_out) != 1) {
			fprintf (stderr, "polygon_update: Write error!\n");
			exit (-1);
		}
		
		if (pol_fwrite (p, h.n, fp_out) != h.n) {
			fprintf (stderr, "polygon_update: Write error!\n");
			exit (-1);
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
