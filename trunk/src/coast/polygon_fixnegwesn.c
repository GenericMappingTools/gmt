/*
 *	$Id: polygon_fixnegwesn.c,v 1.2 2011-04-12 13:06:43 remko Exp $
 */
/* 
 * Add 360 to w/e if both are negative in the header
 */

#include "wvs.h"

int main (int argc, char **argv)
{
	FILE	*fp_in;
	int	k, n_fix = 0;
	struct	LONGPAIR p;
	struct GMT3_POLY h;
        
	if (argc == 1) {
		fprintf(stderr,"usage: polygon_fixnegwesn file.b > new.b\n");
		exit(-1);
	}
	
	fp_in = fopen(argv[1], "r");
		
	while (pol_readheader (&h, fp_in) == 1) {
		if (h.east < 0.0 && h.west< 0.0) {
			fprintf (stderr, "Pol %d has negative w/e values\n", h.id);
			h.east += 360.0;
			h.west += 360.0;
			n_fix++;
		}
		pol_writeheader (&h, stdout);
		for (k = 0; k < h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf(stderr,"polygon_fixnegwesn: Error reading file.\n");
				exit(-1);
			}
			if (pol_fwrite (&p, 1, stdout) != 1) {
				fprintf(stderr,"polygon_fixnegwesn: Error writing file.\n");
				exit(-1);
			}
		}
	}
		
	fclose(fp_in);
	if (n_fix) fprintf(stderr,"polygon_fixnegwesn: Headers corrected = %d\n", n_fix);

	exit (0);
}
