/*
 *	$Id: polygon_to_gshhs.c,v 1.24 2011-03-15 02:06:37 guru Exp $
 * 
 *	read polygon.b format and write a GSHHS file to stdout
 *	For version 1.4 we standardize GSHHS header to only use 4-byte ints.
 *	We also enforce writing of positive longitudes (0-360) * 1e6
 *	Now excludes the extra duplicate point in Antarctica.
 */

#include "wvs.h"
#include "gshhs/gshhs.h"
#define GSHHS_INV_SCL	1.0e6	/* Convert degrees to micro-degrees */

int main (int argc, char **argv)
{
	FILE	*fp_in;
	int	k, version = GSHHS_DATA_RELEASE, lines = 0, np, q = 0;
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
	memset ((void *)&h, 0, sizeof (struct GSHHS));
		
	while (pol_readheader (&h, fp_in) == 1) {
		gshhs_header.west	= irint (h.west * GSHHS_INV_SCL);
		gshhs_header.east	= irint (h.east * GSHHS_INV_SCL);
		gshhs_header.south	= irint (h.south * GSHHS_INV_SCL);
		gshhs_header.north	= irint (h.north * GSHHS_INV_SCL);
		gshhs_header.id		= h.id;
		gshhs_header.n		= h.n;
		gshhs_header.area_full	= (lines) ? 0 : irint (10.0 * h.area);
		gshhs_header.area	= (lines) ? 0 : irint (10.0 * h.area_res);
		gshhs_header.flag	= h.level + (version << 8) + ((h.greenwich & 1) << 16) + (h.source << 24) + (h.river << 25);
		gshhs_header.container	= h.parent;
		gshhs_header.ancestor	= h.ancestor;
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
		gshhs_header.area_full	= swabi4 ((unsigned int)gshhs_header.area_full);
		gshhs_header.flag	= swabi4 ((unsigned int)gshhs_header.flag);
		gshhs_header.container	= swabi4 ((unsigned int)gshhs_header.container);
		gshhs_header.ancestor	= swabi4 ((unsigned int)gshhs_header.ancestor);
#endif
		fwrite((char *)&gshhs_header, sizeof (struct GSHHS), 1, stdout) ;
		for (k = 0; k < h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf (stderr,"polygon_to_gshhs:  ERROR  reading file %s.\n", argv[1]);
				exit (EXIT_FAILURE);
			}
			if (p.x < 0) p.x += M360;
			if (k < np && pol_fwrite2 (&p, 1, stdout) != 1) {
				fprintf (stderr,"polygon_to_gshhs:  ERROR  writing to stdout.\n");
				exit (EXIT_FAILURE);
			}
		}
		q++;
	}
		
	fclose (fp_in);

	exit (EXIT_SUCCESS);
}
