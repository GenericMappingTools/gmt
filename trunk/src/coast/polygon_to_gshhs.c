/*
 *	$Id: polygon_to_gshhs.c,v 1.1 2004-09-05 04:00:51 pwessel Exp $
 */
/* 
 *	read polygon.b format and write a GSHHS file to stdout
 */

#include "wvs.h"

struct GSHHS {
	int id;
	int n;
	int level;			/* 1 land, 2 lake, 3 island_in_lake, etc */
	int west, east, south, north;	/* in micro-degrees */
	int area;			/* Area of polygon in 1/10 km^2 */
	short int greenwich;		/* Greenwich is TRUE if Greenwich is crossed */
	short int source;		/* 0 = CIA WDBII, 1 = WVS */
};

main (int argc, char **argv)
{
	FILE	*fp_in;
	int	k;
	struct	LONGPAIR p;
	struct GMT3_POLY h;
	struct GSHHS h2;
	char file[128];
        
	if (argc != 2) {
		fprintf (stderr,"usage:  polygon_to_gshhs file_res.b > gshhs_res.b\n");
		exit (EXIT_FAILURE);
	}
	
	fp_in = fopen(argv[1], "r");
		
	while (pol_readheader (&h, fp_in) == 1) {
		h2.west = rint (h.west * 1.0e6);
		h2.east = rint (h.east * 1.0e6);
		h2.south = rint (h.south * 1.0e6);
		h2.north = rint (h.north * 1.0e6);
		h2.id = h.id;
		h2.n = h.n;
		h2.greenwich = h.greenwich;
		h2.level = h.level;
		h2.source = h.source;
		h2.area = rint (10.0 * h.area);

		fwrite((char *)&h2, sizeof (struct GSHHS), 1, stdout) ;
		for (k = 0; k < h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf (stderr,"polygon_to_gshhs:  ERROR  reading file.\n");
				exit (EXIT_FAILURE);
			}
			if (fwrite((void *)&p, sizeof(struct LONGPAIR), 1, stdout) != 1) {
				fprintf (stderr,"polygon_to_gshhs:  ERROR  writing file.\n");
				exit(EXIT_FAILURE);
			}
		}
	}
		
	fclose (fp_in);

	exit (EXIT_SUCCESS);
}
