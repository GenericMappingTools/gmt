/*
 *	$Id: polygon_to_gshhs.c,v 1.15 2009-05-28 03:21:53 guru Exp $
 * 
 *	read polygon.b format and write a GSHHS file to stdout
 *	For version 1.4 we standardize GSHHS header to only use 4-byte ints.
 *	We also enforce writing of positive longitudes (0-360) * 1e6
 *	Now excludes the extra duplicate in Antarctica.
 */

#include "wvs.h"
#include "gshhs/gshhs.h"
#define GSHHS_INV_SCL	1.0e6	/* Convert degrees to micro-degrees */

int main (int argc, char **argv)
{
	FILE	*fp_in;
	int	k, version = GSHHS_DATA_VERSION, lines = 0, np;
	struct	LONGPAIR p;
	struct GMT3_POLY h;
	struct GSHHS gshhs_header;
        
	if (argc < 2 || argc > 3) {
		fprintf (stderr,"usage:  polygon_to_gshhs [-l] file_res.b > gshhs_res.b\n");
		fprintf (stderr,"	-l indicates data are lines (rivers, borders) and not polygons\n");
		exit (EXIT_FAILURE);
	}
	if (argc == 3 && !strcmp (argv[1], "-l")) lines = 1;
	fp_in = fopen(argv[1+lines], "r");
		
	while (pol_readheader (&h, fp_in) == 1) {
		gshhs_header.west	= irint (h.west * GSHHS_INV_SCL);
		gshhs_header.east	= irint (h.east * GSHHS_INV_SCL);
		gshhs_header.south	= irint (h.south * GSHHS_INV_SCL);
		gshhs_header.north	= irint (h.north * GSHHS_INV_SCL);
		gshhs_header.id		= h.id;
		gshhs_header.n		= h.n;
		gshhs_header.area	= (lines) ? 0 : irint (10.0 * h.area);
		gshhs_header.flag	= h.level + (version << 8) + (h.greenwich << 16) + (h.source << 24);
		gshhs_header.parent	= h.parent;
		gshhs_header.unused	= 0;
		if ((gshhs_header.east - gshhs_header.west) == M360) gshhs_header.n--;	/* Antarctica, drop the duplicated point for GSHHS */
		np = gshhs_header.n;
#if WORDS_BIGENDIAN == 0
		/* Must swap header explicitly on little-endian machines */
		gshhs_header.west	= swabi4 ((unsigned int)gshhs_header.west);
		gshhs_header.east	= swabi4 ((unsigned int)gshhs_header.east);
		gshhs_header.south	= swabi4 ((unsigned int)gshhs_header.south);
		gshhs_header.north	= swabi4 ((unsigned int)gshhs_header.north);
		gshhs_header.id		= swabi4 ((unsigned int)gshhs_header.id);
		gshhs_header.n		= swabi4 ((unsigned int)gshhs_header.n);
		gshhs_header.area	= swabi4 ((unsigned int)gshhs_header.area);
		gshhs_header.flag	= swabi4 ((unsigned int)gshhs_header.flag);
		gshhs_header.parent	= swabi4 ((unsigned int)gshhs_header.parent);
		gshhs_header.unused	= swabi4 ((unsigned int)gshhs_header.unused);
#endif
		fwrite((char *)&gshhs_header, sizeof (struct GSHHS), 1, stdout) ;
		for (k = 0; k < np; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf (stderr,"polygon_to_gshhs:  ERROR  reading file %s.\n", argv[1]);
				exit (EXIT_FAILURE);
			}
			if (p.x < 0) p.x += M360;
			if (pol_fwrite (&p, 1, stdout) != 1) {
				fprintf (stderr,"polygon_to_gshhs:  ERROR  writing to stdout.\n");
				exit (EXIT_FAILURE);
			}
		}
	}
		
	fclose (fp_in);

	exit (EXIT_SUCCESS);
}
