/*
 *	$Id: polygon_to_gshhs.c,v 1.4 2004-09-28 18:25:21 pwessel Exp $
 * 
 *	read polygon.b format and write a GSHHS file to stdout
 */

#include "wvs.h"
#include "gshhs/gshhs.h"

main (int argc, char **argv)
{
	FILE	*fp_in;
	int	k;
	struct	LONGPAIR p;
	struct GMT3_POLY h;
	struct GSHHS gshhs_header;
        
	if (argc != 2) {
		fprintf (stderr,"usage:  polygon_to_gshhs file_res.b > gshhs_res.b\n");
		exit (EXIT_FAILURE);
	}
	
	fp_in = fopen(argv[1], "r");
		
	while (pol_readheader (&h, fp_in) == 1) {
		gshhs_header.west  = rint (h.west * 1.0e6);
		gshhs_header.east  = rint (h.east * 1.0e6);
		gshhs_header.south = rint (h.south * 1.0e6);
		gshhs_header.north = rint (h.north * 1.0e6);
		gshhs_header.id = h.id;
		gshhs_header.n  = h.n;
		gshhs_header.greenwich = h.greenwich;
		gshhs_header.level  = h.level;
		gshhs_header.version  = 3;
		gshhs_header.source = h.source;
		gshhs_header.area   = rint (10.0 * h.area);
#ifndef WORDS_BIGENDIAN
		/* Must swap header explicitly on little-endian machines */
		gshhs_header.id = swabi4 ((unsigned int)gshhs_header.id);
		gshhs_header.n  = swabi4 ((unsigned int)gshhs_header.n);
		gshhs_header.level = swabi4 ((unsigned int)gshhs_header.level);
		gshhs_header.west  = swabi4 ((unsigned int)gshhs_header.west);
		gshhs_header.east  = swabi4 ((unsigned int)gshhs_header.east);
		gshhs_header.south = swabi4 ((unsigned int)gshhs_header.south);
		gshhs_header.north = swabi4 ((unsigned int)gshhs_header.north);
		gshhs_header.area  = swabi4 ((unsigned int)gshhs_header.area);
		gshhs_header.version  = swabi4 ((unsigned int)gshhs_header.version);
		gshhs_header.greenwich = swabi2 ((unsigned int)gshhs_header.greenwich);
		gshhs_header.source = swabi2 ((unsigned int)gshhs_header.source);
#endif
		fwrite((char *)&gshhs_header, sizeof (struct GSHHS), 1, stdout) ;
		for (k = 0; k < h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf (stderr,"polygon_to_gshhs:  ERROR  reading file %s.\n", argv[1]);
				exit (EXIT_FAILURE);
			}
			if (pol_fwrite (&p, 1, stdout) != 1) {
				fprintf (stderr,"polygon_to_gshhs:  ERROR  writing to stdout.\n");
				exit (EXIT_FAILURE);
			}
		}
	}
		
	fclose (fp_in);

	exit (EXIT_SUCCESS);
}
