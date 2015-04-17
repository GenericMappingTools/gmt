#!/bin/bash
#
#       $Id$

ps=gspline_3.ps

# Figure 4 in Wessel, P. (2009), A general-purpose Green's function-based
#	interpolator, Computers & Geosciences, 35, 1247-1254.

# Create synthetic data used in Lancaster & Salkauskas using C code on the fly

D=0.04
method=r
tension=0.998
cat << EOF > ks.c
#include <stdio.h>
#include <math.h>

int main () {
	double x, y, z, d, r;

	while (scanf ("%lf %lf", &x, &y) != EOF) {
		d = y - x;
		if (d >= 0.5)
			z = 1.0;
		else if (d >= 0.0 && d <= 0.5)
			z = 2.0 * d;
		else {
			r = hypot (x - 1.5, y - 0.5);
			z = (r <= 0.25) ? 0.5 * (cos (4.0 * M_PI * r) + 1.0) : -0.01;
		}
		printf ("%g\t%g\t%g\n", x, y, z);
	}
}
EOF
# Make figure 4a showing the synthetic input data gmt surface and sample points
gcc ks.c -lm -o ks
gmt grdmath -R0/2/0/1 -I$D -r 0 = z.nc
gmt grdmath -R0/2/0/1 -I0.01 -r 0 = zfine.nc
gmt makecpt -Crainbow -T0/1/0.2 > tt.cpt
gmt grd2xyz z.nc | cut -f1,2 | ./ks | gmt xyz2grd -R0/2/0/1 -I$D -Gls.nc -r
gmt grd2xyz zfine.nc | cut -f1,2 | ./ks | gmt xyz2grd -R0/2/0/1 -I0.01 -Glsfine.nc -r
gmt grdview ls.nc -Jx2.5i -JZ1.75 -p155/30 -Ctt.cpt -Wc1p -Qm/lightgray -Bx0.5 -By0.2 -Bz0.2 -BwSEnZ -K --PS_SCALE_X=0.8 --PS_SCALE_Y=0.8 > $ps

gmt blockmean random.xyz -R -I$D > use.xyz
gmt grdcontour lsfine.nc -Jx2.8i -O -Bx0.5 -By0.2 -BWSne -C0.2 -A0.2 -K -Y4.25i -GlLT/1/0,1.5/0.5/1.5/1 >> $ps
gmt psxy use.xyz -R -J -O -K -Sc0.05i -Gblack >> $ps
echo "0 1 a)" | gmt pstext -R -J -O -K -N -F+jBR+f24p -Dj0.1i/0.3i >> $ps
# For Fig 4b, use the sampled points to grid with regularized spline in tension
gmt greenspline use.xyz -R0/2/0/1 -I$D -Gsplined.nc -S${method}${tension} -D1
gmt greenspline use.xyz -R0/2/0/1 -I0.01 -Ggrad.nc -S${method}${tension} -Q135 -D1
gmt grdview splined.nc -Jx2.5i -JZ1.75 -p155/30 -Ctt.cpt -Wc1p -Qm/lightgray -O -Bx0.5 -By0.2 -Bz0.2 -BwSEnZ -K -X6.5i -Y-4.25i >> $ps
gmt makecpt -Cpolar -T-5/5/0.2 -D -Z > tt.cpt
gmt grdimage grad.nc -Ctt.cpt -Jx2.8i -M -Bx0.5 -By0.2 -BWSne -Y4.25i -O -K >> $ps
gmt grdcontour splined.nc -Jx2.8i -O -Bx0.5 -By0.2 -BWSne -C0.2 -A0.2 -K -GlLT/1/0,1.5/0.5/1.5/1 >> $ps
gmt psxy use.xyz -R -J -O -K -Sc0.05i -Gblack >> $ps
echo "0 1 b)" | gmt pstext -R -J -O -K -N -F+jBR+f24p -Dj0.1i/0.3i >> $ps
gmt psxy -R -J -O -T >> $ps

