/*
 *	$Id: polygon_set.c,v 1.3 2011-04-12 13:06:42 remko Exp $
 */
/* polygon_set 
 *
 */

#include "wvs.h"

struct GMT3_POLY hin;

struct LONGPAIR p[N_LONGEST];

int main (int argc, char **argv)
{
	FILE	*fp;
	int	i, id, level, reverse;

	if (argc != 4) {
		fprintf(stderr,"usage: polygon_set final.b id level\n");
		exit(-1);
	}

	fp = fopen(argv[1], "r+");
	id = atoi (argv[2]);
	level = atoi (argv[3]);
	
	while (pol_readheader (&hin, fp) == 1 && hin.id != id) {
		
		if (fseek (fp, hin.n * sizeof (struct LONGPAIR), 1)) {
			fprintf (stderr, "polygon_set: Failed seeking ahead\n");
			exit (-1);
		}
	}
	
	if (hin.id != id) {
		fprintf (stderr, "polygon_set: Could not find polygon # %d in file %s\n", id, argv[1]);
		exit (-1);
	}
	
	if (hin.level == level) {
		fprintf (stderr, "polygon_set: polygon # %d already has level = %d\n", id, level);
		exit (-1);
	}
	
	fprintf (stderr, "polygon_set: Change level from %d to %d for polygon # %d\n", hin.level, level, id);
	
	reverse = (abs (level - hin.level)%2);
	
	hin.level = level;
	
	if (fseek (fp, -sizeof (struct GMT3_POLY), 1)) {
		fprintf (stderr, "polygon_set: Failed seeking backwards\n");
		exit (-1);
	}
	
	if (pol_writeheader (&hin, fp) != 1) {
		fprintf (stderr, "polygon_set: Failed writing header\n");
		exit (-1);
	}
	
	if (!reverse) {
		fclose (fp);
		exit (0);
	}
	
	if (pol_fread (p, hin.n, fp) != hin.n) {
		fprintf(stderr,"polygon_set: Error reading file.\n");
		exit(-1);
	}

	if (fseek (fp, -hin.n * sizeof (struct LONGPAIR), 1)) {
		fprintf (stderr, "polygon_set: Failed seeking backwards\n");
		exit (-1);
	}
	
	for (i = hin.n - 1; i >= 0; i--) {
		if (pol_fwrite (&p[i], 1, fp) != 1) {
			fprintf(stderr,"polygon_set: Error writing file.\n");
			exit(-1);
		}
	}
			
	fclose(fp);

	exit(0);
}
