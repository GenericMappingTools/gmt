/*
 *	$Id: polygon_get.c,v 1.3 2011-04-12 13:06:43 remko Exp $
 */
/* 
 */

#include "wvs.h"

int main (int argc, char **argv)
{
	FILE	*fp_in;
	int	k, level = 0;
	struct	LONGPAIR p;
	struct GMT3_POLY h;
	double w, e, s, n;
        
	if (argc == 1) {
		fprintf(stderr,"usage: polygon_get file.b level w e s n\n");
		exit(-1);
	}
	level = atoi (argv[2]);
	w = atof (argv[3]);
	e = atof (argv[4]);
	s = atof (argv[5]);
	n = atof (argv[6]);
	
	
	fp_in = fopen(argv[1], "r");
		
	while (pol_readheader (&h, fp_in) == 1) {
		if (level > 0 && h.level != level) {
			fseek (fp_in, h.n * sizeof(struct LONGPAIR), 1);
			continue;
		}
		
		if ((h.south > n || h.north < s) || (h.west > e || h.east < w)) {
			fseek (fp_in, h.n * sizeof(struct LONGPAIR), 1);
			continue;
		}
		
		pol_writeheader (&h, stdout);
		for (k = 0; k < h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf(stderr,"polygon_get: Error reading file.\n");
				exit(-1);
			}
			if (pol_fwrite (&p, 1, stdout) != 1) {
				fprintf(stderr,"polygon_get: Error writing file.\n");
				exit(-1);
			}
		}
	}
		
	fclose(fp_in);

	exit (0);
}
