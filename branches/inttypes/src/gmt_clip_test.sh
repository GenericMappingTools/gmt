#!/bin/sh
#	$Id$
# Test program for new rectangular clipping.  We create N random points
# in the x/y plane with mean 0 and sigma = 3.  Then, sort these points
# according to the angle they make with (0,0) so we get a non-intersecting
# polygon.  Now impose the clipping on the +-5 rectangle.  We write out
# the original data to clip_data.d and send the clipped polygon to stdout.
# The rest of the script makes the figure and views it via gv.
#

# C code driver:

cat << EOF > gmt_clip_test.c
#include "gmt.h"
#define	N 25
#define HALF    5.0
struct STUFF {
	double x, y, a;
} P[N];

GMT_LONG GMT_rect_clip (double *lon, double *lat, GMT_LONG n, double **x, double **y, int *total_nx);

int compare (const void *point_1, const void *point_2)
{
        struct STUFF *p1, *p2;
        p1 = (struct STUFF *)point_1;
        p2 = (struct STUFF *)point_2;
        if (p1->a < p2->a) return (-1);
        if (p1->a > p2->a) return (1);
        return (0);
}

int main (int argc, char **argv) {
	int n = N, m, j, i, nx = 0;
	double xin[N], yin[N], x, y, *xout, *yout;
	FILE *fp;
	
	argc = GMT_begin (argc, argv);
	for (i = 0; i < N; i++) {	/* Create N random points */
		P[i].x = 3.0 * GMT_nrand ();
		P[i].y = 3.0 * GMT_nrand ();
		P[i].a = atan2 (P[i].y, P[i].x);	/* Get angle */
	}
	qsort ((void *)P, (size_t) N, sizeof (struct STUFF), compare);	/* Sort on increasing angle */
	fp = fopen ("clip_input.d", "w");
	for (i = 0; i < N; i++) {
		xin[i] = P[i].x;	yin[i] = P[i].y;
		fprintf (fp, "%g\t%g\n", xin[i], yin[i]);
	}
	fclose (fp);
		/* Supply dummy linear proj -Jx1 */
	project_info.projection = project_info.xyz_projection[0] = project_info.xyz_projection[1] = GMT_LINEAR;
	project_info.pars[0] = project_info.pars[1] = 1.0;
	GMT_err_fail (GMT_map_setup (-HALF, HALF, -HALF, HALF), "");
	m = GMT_rect_clip (xin, yin, (GMT_LONG)n, &xout, &yout, &nx);
		for (j = 0; j < m; j++) {
		GMT_xy_to_geo (&x, &y, xout[j], yout[j]);
		printf ("%g\t%g\n", x, y);
	}
	GMT_free ((void *)xout);	GMT_free ((void *)yout);
	exit (EXIT_SUCCESS);
}
EOF

# Test script

make gmt_clip_test.o
gcc gmt_clip_test.o -L. -lgmt -lgmtps -lpsl -L/sw/lib -lnetcdf -o gmt_clip_test

gmt_clip_test > clip_output.d
psxy -R-10/10/-10/10 -JX8 -P -B2WSne clip_input.d -L -P -K  -X0.25i -Y0.5i > clip_test.ps
psxy -R -J -O -K clip_input.d -Sc0.1i -Ggreen -Wfaint >> clip_test.ps
psxy -R -J -O -K clip_output.d -L -W0.5p,red >> clip_test.ps
psxy -R -J -O -K clip_output.d -Sc0.05i -Gred >> clip_test.ps
psxy -R -J -O -L -W0.5p,blue << EOF >> clip_test.ps
-5	-5
5	-5
5	5
-5	5
EOF
gv clip_test.ps &
