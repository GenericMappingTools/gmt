/*
 *	$Id$
 */
/* 
 *
 * polygon_id returns the id of the segment that comes closest to the
 * specificed lon,lat point.  The min distance (in m) is also returned.
 */

#include "wvs.h"

#define D2M	(2*6378132.0*M_PI/360.0)

struct GMT3_POLY h;

int main (int argc, char **argv)
{
	FILE	*fp_in;
	int	k;
	struct	LONGPAIR p;
	double w, e, s, n;
        
	if (argc == 1) {
		fprintf(stderr,"usage: polygon_id polygons.b w e s n\n");
		exit(-1);
	}

	fp_in = fopen(argv[1], "r");
	w = atof (argv[2]);
	e = atof (argv[3]);
	s = atof (argv[4]);
	n = atof (argv[5]);
		
	while (pol_readheader (&h, fp_in) == 1) {
		for (k = 0; k < h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf(stderr,"polygon_id: Error reading file.\n");
				exit(-1);
			}
		}
		if (h.west > w && h.east < e && h.south > s && h.north < n) printf ("%d\n", h.id);
	}
	
	fclose(fp_in);

	exit (0);
}
