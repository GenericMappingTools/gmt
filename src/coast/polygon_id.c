/*
 *	$Id: polygon_id.c,v 1.5 2011-04-12 13:06:43 remko Exp $
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
	int	id = 0, k;
	struct	LONGPAIR p;
	double x0, y0, d_min, d, dx, dy;
        
	if (argc == 1) {
		fprintf(stderr,"usage: polygon_id polygons.b lon0 lat0\n");
		exit(-1);
	}

	fp_in = fopen(argv[1], "r");
	x0 = atof (argv[2]);
	y0 = atof (argv[3]);
		
	d_min = 1.0e38;
	if (x0 < 0.0) x0 += 360.0;
	
	while (pol_readheader (&h, fp_in) == 1) {
		for (k = 0; k < h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf(stderr,"polygon_id: Error reading file.\n");
				exit(-1);
			}
			
			dx = 1.0e-6*p.x - x0;
			dy = 1.0e-6*p.y - y0;
			d = hypot (dx, dy);
			if (d < d_min) {
				d_min = d;
				id = h.id;
			}
		}
	}
	
	printf ("polygon_id: Min dist = %.1f m for polygon id %d\n", D2M * d_min, id);
	
	fclose(fp_in);

	exit (0);
}
