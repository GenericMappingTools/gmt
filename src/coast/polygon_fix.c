/*
 *	$Id: polygon_fix.c,v 1.1 2004-09-05 04:00:51 pwessel Exp $
 */
/* 
 */

#include "wvs.h"

main (argc, argv)
int	argc;
char **argv;
{
	FILE	*fp_in;
	int	k, level = 0, first_id = 0;
	struct	LONGPAIR p;
	struct GMT3_POLY h;
	char file[80];
        
	if (argc == 1) {
		fprintf(stderr,"usage:  polygon_fix file.b level first_id > new.b\n");
		exit(-1);
	}
	level = atoi (argv[2]);
	first_id = atoi (argv[3]);
	
	fp_in = fopen(argv[1], "r");
		
	while (pol_readheader (&h, fp_in) == 1) {
		h.level = level;
		h.id = first_id++;
		pol_writeheader (&h, stdout);
		for (k = 0; k < h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf(stderr,"polygon_fix:  ERROR  reading file.\n");
				exit(-1);
			}
			if (pol_fwrite (&p, 1, stdout) != 1) {
				fprintf(stderr,"polygon_fix:  ERROR  writing file.\n");
				exit(-1);
			}
		}
	}
		
	fclose(fp_in);
	fprintf(stderr,"polygon_fix:  Next id = %d\n", first_id);

	exit (0);
}
