/*
 *	$Id: polygon_consistency.c,v 1.3 2004-09-10 04:28:46 pwessel Exp $
 */
/* polygon_consistency checks for propoer closure and crossings
 * within polygons
 *
 */

#include "wvs.h"

double lon[N_LONGEST], lat[N_LONGEST];

main (int argc, char **argv)
{
	FILE	*fp;
	int	i, n_id, this_n, nx, n_x_problems, n_c_problems, n_r_problems, ix0, iy0;
	int w, e, s, n, ixmin, ixmax, iymin, iymax, ANTARCTICA;
	struct GMT_XSEGMENT *ylist;
	struct GMT_XOVER XC;
	struct GMT3_POLY h;
	struct LONGPAIR p;

	if (argc != 2) {
		fprintf(stderr,"usage:  polygon_consistency wvs_polygons.b > report.lis\n");
		exit(-1);
	}

	fp = fopen(argv[1], "r");
	
	n_id = n_c_problems = n_x_problems = n_r_problems = 0;
	while (pol_readheader (&h, fp) == 1) {
		ANTARCTICA = (fabs (h.east - h.west) == 360.0);
		ixmin = iymin = M360;
		ixmax = iymax = -M360;
		w = irint (h.west * 1e6);
		e = irint (h.east * 1e6);
		s = irint (h.south * 1e6);
		n = irint (h.north * 1e6);
		for (i = 0; i < h.n; i++) {
			if (pol_fread (&p, 1, fp) != 1) {
				fprintf(stderr,"polygon_consistency:  ERROR  reading file.\n");
				exit(-1);
			}
			if (h.greenwich && p.x > h.datelon) p.x -= M360;
			if (i == 0) {
				ix0 = p.x;
				iy0 = p.y;
			}
			lon[i] = p.x * 1e-6;
			lat[i] = p.y * 1e-6;
			if (p.x < ixmin) ixmin = p.x;
			if (p.x > ixmax) ixmax = p.x;
			if (p.y < iymin) iymin = p.y;
			if (p.y > iymax) iymax = p.y;
		}
		
		if (ANTARCTICA) iymin = -M90;
		if (! (p.x == ix0 && p.y == iy0)) {
			printf ("%d\tnot closed\n", h.id);
			n_c_problems++;
			this_n = h.n;
		}
		else
			this_n = h.n - 1;
		if (! (ixmin == w && ixmax == e && iymin == s && iymax == n)) {
			printf ("%d\twesn mismatch\n", h.id);
			n_r_problems++;
		}
		ylist = GMT_init_track (lon, lat, this_n);
		if (fabs (h.east - h.west) > GMT_CONV_LIMIT) {
			nx = GMT_crossover (lon, lat, NULL, ylist, this_n, lon, lat, NULL, ylist, this_n, TRUE, &XC);
			GMT_free ((void *)ylist);
			if (nx ) {
				for (i = 0; i < nx; i++) printf ("%d\t%d\t%10.5lf\t%9.5lf\n", h.id, (int)floor(XC.xnode[0][i]), XC.x[i], XC.y[i]);
				n_x_problems++;
				GMT_x_free (&XC);
			}
		}
		n_id++;
	}
	
	fprintf (stderr, "polygon_consistency: Got %d polygons from file %s. %d has closure problems. %d has crossing problems. %d has region problems\n",
		n_id, argv[1], n_c_problems, n_x_problems, n_r_problems);

	fclose(fp);

	exit(0);
}

