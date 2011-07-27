/*
 *	$Id$
 */
/* polygon_consistency checks for propoer closure and crossings
 * within polygons
 *
 */

#include "wvs.h"

double lon[N_LONGEST], lat[N_LONGEST];
#define P_LIMIT 0.001

int main (int argc, char **argv)
{
	FILE	*fp;
	int	i, n_id, this_n, nd, nx, n_x_problems, n_s_problems, n_c_problems, n_r_problems, n_b_problems;
	int	n_d_problems, n_a_problems, n_p_problems, ix0 = 0, iy0 = 0, report_mismatch = 0, cont_no;
	int	w, e, s, n, ixmin, ixmax, iymin, iymax, last_x = 0, last_y = 0;
	int	ant_trouble = 0, found, A, B, end, n_adjust = 0, left, right;
	struct GMT_XSEGMENT *ylist;
	struct GMT_XOVER XC;
	struct GMT3_POLY h;
	struct LONGPAIR p;
	double dx1, dx2, dy1, dy2, off, cos_a;

	if (argc != 2) {
		fprintf(stderr,"usage: polygon_consistency wvs_polygons.b > report.lis\n");
		exit(-1);
	}

	fp = fopen(argv[1], "r");
	
	n_id = n_c_problems = n_x_problems = n_r_problems = n_d_problems = n_s_problems = n_a_problems = n_b_problems = n_p_problems = 0;
	while (pol_readheader (&h, fp) == 1) {
		if (n_id == 0 && h.n > 1000000) report_mismatch = 1;
		if (h.id == 13401)
			w = 0;
	
		cont_no = (h.river >> 8);	/* Get continent nubmer 1-6 (0 if not a continent) */
		if (cont_no == ANTARCTICA) {
			if (h.south > -90.0) ant_trouble = TRUE;
		}
		if (h.area < 0 && h.level != 2) fprintf (stderr, "Pol %d has negative area and is level %d\n", h.id, h.level);
		if ((h.river & 1) && h.level != 2) fprintf (stderr, "Pol %d is a riverlake but level is %d\n", h.id, h.level);
		if (GMT_IS_ZERO (h.area_res)) {
			fprintf (stderr, "Pol %d has zero resolution area\n", h.id);
			n_b_problems++;
		}
		if ((h.area_res / fabs(h.area)) < P_LIMIT) {	/* Less than 0.01% of original area */
			fprintf (stderr, "Pol %d has < %g %% of full resolution area\n", h.id, 100.0 * P_LIMIT);
			n_p_problems++;
		}
		ixmin = iymin = M360;
		ixmax = iymax = -M360;
		w = irint (h.west * 1e6);
		e = irint (h.east * 1e6);
		s = irint (h.south * 1e6);
		n = irint (h.north * 1e6);
		for (i = nd = 0; i < h.n; i++) {
			if (pol_fread (&p, 1, fp) != 1) {
				fprintf(stderr,"polygon_consistency: Error reading file.\n");
				exit(-1);
			}
			if (p.x < 0 || p.x > M360) fprintf (stderr, "Pol %d, point %d: x outside range [%g]\n", h.id, i, p.x*1e-6);
			if (p.y < -M90 || p.y > M90) fprintf (stderr, "Pol %d, point %d: y outside range [%g]\n", h.id, i, p.y*1e-6);
			if ((h.greenwich & 1) && p.x > h.datelon) p.x -= M360;
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
			if (i > 0 && last_x == p.x && last_y == p.y) {
				printf ("%d\tduplicate point line %d\n", h.id, i);
				nd++;
			}
			last_x = p.x;
			last_y = p.y;
		}
		
		if (nd) n_d_problems++;
		if (cont_no == ANTARCTICA) iymin = -M90;
		if (! (p.x == ix0 && p.y == iy0)) {
			printf ("%d\tnot closed\n", h.id);
			n_c_problems++;
		}
		if (report_mismatch && !(ixmin == w && ixmax == e && iymin == s && iymax == n)) {
			printf ("%d\twesn mismatch.  Should be %.6f/%.6f/%.6f/%.6f\n", h.id, 1e-6 * ixmin, 1e-6 * ixmax, 1e-6 * iymin, 1e-6 * iymax);
			n_r_problems++;
		}
		this_n = (cont_no == ANTARCTICA) ? h.n - 1 : h.n;
		GMT_init_track (lat, this_n, &ylist);
		if (!GMT_IS_ZERO (h.east - h.west)) {
			nx = found = GMT_crossover (lon, lat, NULL, ylist, this_n, lon, lat, NULL, ylist, this_n, TRUE, &XC);
			GMT_free ((void *)ylist);
			for (i = end = 0; i < nx; i++) {
				A = irint (XC.xnode[0][i]);
				B = irint (XC.xnode[1][i]);
				if ((A == 0 && B == (h.n-1)) || (B == 0 && A == (h.n-1))) {	/* Involving end point */
					off = MAX (fabs((double)A - XC.xnode[0][i]), fabs((double)B - XC.xnode[1][i]));
					if (GMT_IS_ZERO (off)) {
						/* Remove the crossover caused by the duplicate start/end points */
						end++;
						n_adjust++;
					}
				}
			}
			nx -= end;

			if (nx) {
				for (i = 0; i < nx; i++) printf ("P = %d\tnode = %d\t%10.5f\t%9.5f\n", h.id, (int)floor(XC.xnode[0][i]), XC.x[i], XC.y[i]);
				n_x_problems++;
			}
			if (found) GMT_x_free (&XC);
		}
		/* Look for duplicate points separated by a single outlier (a non-area peninsula) */
		
		for (i = 0; i < (h.n-1); i++) {
			left = (i) ? i - 1 : h.n - 2;	/* Skip around and avoid duplicate end point */
			right = i + 1;			/* Never wrap since we dont go to the end point */
			if (GMT_IS_ZERO (lon[left]-lon[right]) && GMT_IS_ZERO (lat[left]-lat[right])) {
				printf ("%d\tNon-area excursion on line %d-%d-%d\n", h.id, left, i, right);
				n_s_problems++;
			}
			/* Also check for 3 points on a line with zero angle between them */
			dy1 = lat[left] - lat[i];
			dy2 = lat[right] - lat[i];
			dx1 = lon[left] - lon[i];
			dx2 = lon[right] - lon[i];
			if (dy1 == 0.0 && dy2 == 0.0) {	/* Horizontal line, check x arrangement */
				if ((dx1 * dx2) > 0.0) {
					printf ("%d\tZero-angle excursion on line %d-%d-%d\n", h.id, left, i, right);
					n_a_problems++;
				}
			}
			else if (dx1 == 0.0 && dx2 == 0.0) {	/* Vertical line, check y arrangement */
				if ((dy1 * dy2) > 0.0) {
					printf ("%d\tZero-angle excursion on line %d-%d-%d\n", h.id, left, i, right);
					n_a_problems++;
				}
			}
			else {
				double t;
				cos_a = (dx1 * dx2 + dy1 * dy2) / (hypot (dx1, dy1) * hypot (dx2, dy2));	/* Normalized dot product of vectors from center node to the 2 neightbors */
				t = 1.0 - cos_a;
				if (GMT_IS_ZERO (fabs (t))) {
					printf ("%d\tZero-angle excursion on line %d-%d-%d\n", h.id, left, i, right);
					n_a_problems++;
				}
			}
		}
		n_id++;
	}
	
	fprintf (stderr, "polygon_consistency: Got %d polygons from file %s\n", n_id, argv[1]);
	if (n_b_problems) fprintf (stderr, "%d has no area.\n", n_b_problems);
	if (n_p_problems) fprintf (stderr, "%d has < %g %% of full area.\n", n_p_problems, 100.0 * P_LIMIT);
	if (n_c_problems) fprintf (stderr, "%d has closure problems.\n", n_c_problems);
	if (n_x_problems) fprintf (stderr, "%d has crossing problems.\n", n_x_problems);
	if (n_r_problems) fprintf (stderr, "%d has region problems.\n", n_r_problems);
	if (n_d_problems) fprintf (stderr, "%d has duplicate points.\n", n_d_problems);
	if (n_s_problems) fprintf (stderr, "%d has non-area excursions.\n", n_s_problems);
	if (n_a_problems) fprintf (stderr, "%d has zero-angle excursions\n", n_a_problems);
	if (ant_trouble) fprintf (stderr, "polygon_consistency: Antarctica polygon has wrong south border\n");
	if (n_adjust) fprintf (stderr, "polygon_consistency: Skipped %d crossovers involving duplicate end points (polygon closure)\n", n_adjust);
	
	fclose(fp);

	exit(0);
}
