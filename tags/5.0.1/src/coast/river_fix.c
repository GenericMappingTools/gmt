/*
 *	$Id$
 */
/*  used to set all levels > 4 to level-1 (since 5 was removed)
 */

#include "wvs.h"

int main (int argc, char **argv)
{
	FILE	*fp_in;
	int	k;
	struct	LONGPAIR p;
	struct GMT3_POLY h;
        
	if (argc == 1) {
		fprintf(stderr,"usage: river_fix file.b > new.b\n");
		exit(-1);
	}
	
	fp_in = fopen(argv[1], "r");
		
	while (pol_readheader (&h, fp_in) == 1) {
		if (h.level > 4) h.level--;
		pol_writeheader (&h, stdout);
		for (k = 0; k < h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf(stderr,"polygon_fix: Error reading file.\n");
				exit(-1);
			}
			if (pol_fwrite (&p, 1, stdout) != 1) {
				fprintf(stderr,"polygon_fix: Error writing file.\n");
				exit(-1);
			}
		}
	}
		
	fclose(fp_in);

	exit (0);
}
