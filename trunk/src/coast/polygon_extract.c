/*
 *	$Id: polygon_extract.c,v 1.1 2004-09-05 04:00:51 pwessel Exp $
 */
/* 
 *
 */

#include "wvs.h"

struct CHECK {
	int pos;
	struct GMT3_POLY h;
} poly[N_POLY];

main (int argc, char **argv)
{
	FILE	*fp_in, *fp;
	int	i, j, id, n_id, k, pos;
	struct	LONGPAIR p;
	char file[80];
        
	if (argc == 1) {
		fprintf(stderr,"usage:  polygon_extract final_polygons.b id1 id2 id3 ... idn\n");
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
	
	/* fp = fopen ("/home/aa4/gmt/wvs/headers3.b", "r");
	fread ((char *)&n_id, sizeof (int), 1, fp);
	fread ((char *)poly, sizeof (struct CHECK), n_id, fp);
	
	fp = fopen ("/home/aa4/gmt/wvs/headers3.b", "w");
	fwrite ((char *)&n_id, sizeof (int), 1, fp);
	fwrite ((char *)poly, sizeof (struct CHECK), n_id, fp);
	fclose (fp); */
	
	/* Start extraction */
	
	for (i = 2; i < argc; i++) {
		id = atoi (argv[i]);
		
		if (id < 0) continue;
		
		fprintf (stderr, "Extracting Polygon # %d\n", id);	
				
		for (j = 0; j < n_id && poly[j].h.id != id; j++);
		
		/* j = id; */
		
		if (j == n_id) continue;
		
		sprintf (file, "polygon.%d\0", id);
		fp = fopen (file, "w");
		
		fseek (fp_in, poly[j].pos, SEEK_SET);
		
		for (k = 0; k < poly[j].h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf(stderr,"polygon_extract:  ERROR  reading file.\n");
				exit(EXIT_FAILURE);
			}
			if (poly[j].h.greenwich && p.x > poly[j].h.datelon) p.x -= M360;
			
			/* fprintf (fp, "%d\t%d\n", p.x, p.y); */
			fprintf (fp, "%.10lg\t%.10lg\n", 1.0e-6*p.x, 1.0e-6*p.y);
		}
		fclose (fp);
		
	}
		
	fclose (fp_in);

	exit (EXIT_SUCCESS);
}
