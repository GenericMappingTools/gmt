/*
 *	$Id$
 */
/* 
 */

#include "wvs.h"

int main (int argc, char **argv)
{
	FILE	*fp_in;
	int	new_level = 0, id = 0, old_level;
	struct GMT3_POLY h;
        
	if (argc != 4) {
		fprintf (stderr, "usage: polygon_fixlevel database.b id new_level\n");
		fprintf (stderr, "Note: polygon_fixlevel modifies the database.b directly!\n");
		exit (-1);
	}

	id = atoi (argv[2]);
	new_level = atoi (argv[3]);
	
	if ((fp_in = fopen(argv[1], "r+")) == NULL ) {
		fprintf (stderr, "polygon_fixlevel: Could not open %s for read/write!\n", argv[1]);
		exit (-1);
	}
		
	while (pol_readheader (&h, fp_in) == 1 && h.id != id) {
		fseek (fp_in, (long) (h.n * sizeof (struct LONGPAIR)), SEEK_CUR);
		fflush (fp_in);
	}
	
	old_level = h.level;

	if (old_level == new_level) {
		fprintf(stderr,"polygon_fixlevel: Polygon %d already has level = %d!\n", id, old_level);
		exit (-1);
	}

	/* Here we must do something.  First replace header */

	h.level = new_level;
	if (fseek (fp_in, (long) (-sizeof (struct GMT3_POLY)), SEEK_CUR)) {
 		fprintf(stderr,"polygon_fixlevel: Error seeking to start of header.\n");
		exit(-1);
	}
	fflush (fp_in);

	if (pol_writeheader (&h, fp_in) != 1) {
		fprintf(stderr,"polygon_fixlevel: Error writing file.\n");
		exit(-1);
	}
	fflush (fp_in);

#ifdef REVERSE
	if (abs (new_level - old_level) % 2 == 1) {	/* Must reverse polygon */
		p = (struct LONGPAIR *) GMT_memory (CNULL, h.n, sizeof (struct LONGPAIR), "polygon_fixlevel");
		if (pol_fread (p, h.n, fp_in) != h.n) {
			fprintf(stderr,"polygon_fixlevel: Error reading file.\n");
			exit(-1);
		}
		fflush (fp_in);
		if (fseek (fp_in, (long) (-h.n * sizeof (struct LONGPAIR)), SEEK_CUR)) {
 			fprintf(stderr,"polygon_fixlevel: Error seeking to start of data.\n");
			exit(-1);
		}
		fflush (fp_in);
		for (k = 0; k < h.n; k++) {
			if (pol_fwrite (&p[h.n-1-k], 1, fp_in)) {
				fprintf(stderr,"polygon_fixlevel: Error writing file.\n");
				exit(-1);
			}
		}
		fflush (fp_in);
	}
#endif
	fclose(fp_in);
	fprintf(stderr,"polygon_fixlevel: Polygon %d successfully fixed\n", id);

	exit (0);
}
