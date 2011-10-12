/*
 *	$Id$
 * 
 *	read a GSHHS file and and write a polygon.b format file to stdout
 * UNTESTED but should work. (-pw)
 */

#include "wvs.h"
#include "gshhs/gshhs.h"

int main (int argc, char **argv)
{
	FILE	*fp_in;
	int	k;
	struct	LONGPAIR p;
	struct GMT3_POLY h;
	struct GSHHS gshhs_header;
        
	if (argc != 2) {
		fprintf (stderr,"usage: gshhs_to_polygon gshhs_res.b > polygonfile_res.b\n");
		exit (EXIT_FAILURE);
	}
	
	fp_in = fopen(argv[1], "rb");
		
	while (fread ((void *)&gshhs_header, sizeof (struct GSHHS), 1, fp_in) == 1) {
#ifndef WORDS_BIGENDIAN
		/* Must swap header explicitly on little-endian machines */
		gshhs_header.id = swabi4 ((unsigned int)gshhs_header.id);
		gshhs_header.n  = swabi4 ((unsigned int)gshhs_header.n);
		gshhs_header.flag = swabi4 ((unsigned int)gshhs_header.flag);
		gshhs_header.west  = swabi4 ((unsigned int)gshhs_header.west);
		gshhs_header.east  = swabi4 ((unsigned int)gshhs_header.east);
		gshhs_header.south = swabi4 ((unsigned int)gshhs_header.south);
		gshhs_header.north = swabi4 ((unsigned int)gshhs_header.north);
		gshhs_header.area  = swabi4 ((unsigned int)gshhs_header.area);
#endif
		h.west = (double) gshhs_header.west * 1.0e-6;
		h.east = (double) gshhs_header.east * 1.0e-6;
		h.south = (double) gshhs_header.south * 1.0e-6;
		h.north = (double) gshhs_header.north * 1.0e-6;
		h.id = gshhs_header.id;
		h.n = gshhs_header.n;
		h.greenwich = (gshhs_header.flag >> 16) & 255;
		h.level = gshhs_header.flag & 255;
		h.source = (gshhs_header.flag >> 24) & 255;
		h.area = gshhs_header.area * 0.1;
		pol_writeheader (&h, stdout);
		for (k = 0; k < h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf (stderr,"gshhs_to_polygon: Error reading file %s.\n", argv[1]);
				exit (EXIT_FAILURE);
			}
			if (pol_fwrite (&p, 1, stdout) != 1) {
				fprintf (stderr,"gshhs_to_polygon: Error writing to stdout.\n");
				exit (EXIT_FAILURE);
			}
		}
	}
		
	fclose (fp_in);

	exit (EXIT_SUCCESS);
}
