/*
 *	$Id: polygon_fix.c,v 1.5 2011-04-12 13:06:43 remko Exp $
 *
 * Used to make minor fixes - check src to see what it actually is doing!
 */
/* 
 */

#include "wvs.h"

int main (int argc, char **argv)
{
	FILE	*fp_in;
	int	k, first_id = 0;
	struct	LONGPAIR p;
	struct GMT3_POLY h;
        
	if (argc == 1) {
		fprintf(stderr,"usage: polygon_fix file.b > new.b\n");
		exit(-1);
	}
	
	fp_in = fopen(argv[1], "r");
		
	while (pol_readheader (&h, fp_in) == 1) {
		h.area = 0.0;
		h.id = first_id++;
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
	fprintf(stderr,"polygon_fix: Next id = %d\n", first_id);

	exit (0);
}
