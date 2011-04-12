/*
 *	$Id: polygon_extract2.c,v 1.4 2011-04-12 13:06:43 remko Exp $
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
	FILE	*fp_in, *fp;
	int	i, id, n_id, k, w, e, s, n;
	struct	LONGPAIR p;
	char file[80];
        
	if (argc == 1) {
		fprintf(stderr,"usage: polygon_extract final_polygons.b w e s n id \n");
		exit(-1);
	}

	fp_in = fopen(argv[1], "r");
	w = atoi (argv[2]) * 1000000;
	e = atoi (argv[3]) * 1000000;
	s = atoi (argv[4]) * 1000000;
	n = atoi (argv[5]) * 1000000;
	id = atoi (argv[6]);
		
	/* n_id = pos = 0;
	while (fread((char *)&poly[n_id].h, sizeof (struct GMT3_POLY), 1, fp_in) == 1) {
		pos += sizeof (struct GMT3_POLY);
		poly[n_id].pos = pos;
		fseek (fp_in, poly[n_id].h.n * sizeof(struct LONGPAIR), 1);
		pos += poly[n_id].h.n * sizeof(struct LONGPAIR);
		poly[n_id].h.id = n_id;
		n_id++;
	}
	fp = fopen ("/home/aa4/gmt/wvs/headers.b", "w");
	fwrite ((char *)&n_id, sizeof (int), 1, fp);
	fwrite ((char *)poly, sizeof (struct CHECK), n_id, fp);
	fclose (fp); */
	fp = fopen ("/home/aa4/gmt/wvs/headers.b", "r");
	fread ((char *)&n_id, sizeof (int), 1, fp);
	fread ((char *)poly, sizeof (struct CHECK), n_id, fp);
	fclose (fp);
	
	/* Start extraction */
	
	fprintf (stderr, "Extracting Polygon # %d\n", id);
	
	for (i = 0; i < n_id && poly[i].h.id != id; i++);	
				
	sprintf (file, "polygon.%d", id);
	fp = fopen (file, "w");
		
	fseek (fp_in, poly[i].pos, 0);
	if ((poly[i].h.greenwich & 1) && w > poly[i].h.datelon) w -= M360, e -= M360;
		
	for (k = 0; k < poly[i].h.n; k++) {
		if (pol_fread (&p, 1, fp_in) != 1) {
			fprintf(stderr,"polygon_extract: Error reading file.\n");
			exit(-1);
		}
		if (p.y < s || p.y > n) continue;
		if ((poly[i].h.greenwich & 1) && p.x > poly[i].h.datelon) p.x -= M360;
		if (p.x < w || p.x > e) continue;
		
		/* fprintf (fp, "%d\t%d\n", p.x, p.y); */
		fprintf (fp, "%g\t%g\n", 1.0e-6*p.x, 1.0e-6*p.y);
	}
	fclose (fp);
		
	fclose(fp_in);

	exit (0);
}
