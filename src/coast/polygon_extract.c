/*
 *	$Id: polygon_extract.c,v 1.12 2011-04-12 13:06:43 remko Exp $
 */
/* 
 *
 */

#include "wvs.h"

struct CHECK {
	int pos;
	struct GMT3_POLY h;
} poly[N_POLY];

int main (int argc, char **argv)
{
	FILE	*fp_in, *fp = NULL;
	int	i, j, id, n_id, k, pos, start = 2, multi = FALSE, bin = FALSE, individual = FALSE;
	struct	LONGPAIR p;
	char file[80];
        
	if (argc == 1) {
		fprintf(stderr,"usage: polygon_extract final_polygons.b [-M|b] id1 id2 id3 ... idn\n");
		fprintf(stderr,"	-M will write a multiseg ascii file to stdout\n");
		fprintf(stderr,"	-b will write binary polygon file to stdout\n");
		exit (EXIT_FAILURE);
	}

	fp_in = fopen (argv[1], "rb");
		
	n_id = pos = 0;
	while (pol_readheader (&poly[n_id].h, fp_in) == 1) {
		pos += sizeof (struct GMT3_POLY);
		poly[n_id].pos = pos;
		fseek (fp_in, poly[n_id].h.n * sizeof(struct LONGPAIR), SEEK_CUR);
		pos += poly[n_id].h.n * sizeof(struct LONGPAIR);
		/* poly[n_id].h.id = n_id; */
		n_id++;
	}
	
	/* Start extraction */
	
	if (!strncmp (argv[2], "-M", 2)) {	/* Just want everything out in a multiseg file */
		start = 3;
		multi = TRUE;
		fp = stdout;
	}
	else if (!strncmp (argv[2], "-b", 2)) {	/* Just want everything out in a binary polygon file */
		start = 3;
		bin = TRUE;
		fp = stdout;
	}
	else
		individual = TRUE;
	
	for (i = start; i < argc; i++) {
		id = atoi (argv[i]);
		
		if (id < 0) continue;
		
		fprintf (stderr, "Extracting Polygon # %d\n", id);	
				
		for (j = 0; j < n_id && poly[j].h.id != id; j++);
		
		/* j = id; */
		
		if (j == n_id) continue;
		
		if (multi)
			fprintf (fp, "> Polygon %d\n", id);
		else if (bin)
			pol_writeheader (&poly[j].h, fp);
		else {
			sprintf (file, "polygon.%d", id);
			fp = fopen (file, "w");
		}
		
		fseek (fp_in, poly[j].pos, SEEK_SET);
		
		for (k = 0; k < poly[j].h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf(stderr,"polygon_extract: Error reading file.\n");
				exit (EXIT_FAILURE);
			}
			if (p.x < 0) fprintf (stderr, "x < 0\n");
			if (bin)
				pol_fwrite (&p, 1, fp);
			else {
				if ((poly[j].h.greenwich & 1) && p.x > poly[j].h.datelon) p.x -= M360;
				fprintf (fp, "%.6f\t%.6f\n", 1.0e-6*p.x, 1.0e-6*p.y);
			}
		}
		if (individual) fclose (fp);
		
	}
		
	fclose (fp_in);

	exit (EXIT_SUCCESS);
}
