#!/usr/bin/env bash
#
# GMT_KNOWN_FAILURE
#
# A contour must be drawn at the level it is labelled with, whatever the local
# gradient of the grid is.  This test fails on the GDAL tracer path and passes
# with gmt_contours().
#
# grdcontour computes a noise threshold
#
#	small = (C interval > 0) ? MIN (C interval, z_range) * 1e-6 : z_range * 1e-6
#
# and gmt_contours() used it as
#
#	G = G_orig - cval;  if (G == 0) G += small;
#
# i.e. it nudged only the nodes whose value was EXACTLY the contour level, so
# that no contour ran through a node, and left the contour's position alone.
# The GDAL path instead hands the tracer cval - small as the level, which moves
# the whole contour by small / |grad z|.  That is invisible on a steep grid and
# unbounded on a flat one.
#
# Here z = exp(-x) over 0/20, so z_range is 1 and, because the level comes from
# a contour FILE, the C interval is zero and small is the full z_range * 1e-6 =
# 1e-6.  The wanted level is 3e-6, whose analytic position is
#
#	x = -ln (3e-6) = 12.7168983
#
# while the level actually traced, 3e-6 - 1e-6 = 2e-6, sits at
#
#	x = -ln (2e-6) = 13.1223634
#
# 0.405 away, or eight grid cells.  Linear interpolation across one 0.05 cell of
# an exponential is good to 3e-4 in x, so half a cell is a generous tolerance.

rm -f fail

echo "0.000003 C" > cont.txt
gmt grdmath -R0/20/0/1 -I0.05 X NEG EXP = decay.nc
gmt grdcontour decay.nc -Ccont.txt -JX10c/2c -Ddecay.txt

$AWK '
BEGIN { want = 12.7168983; tol = 0.025 }
/^>/ { nseg++; next }
NF >= 3 {
	d = $1 - want; if (d < 0) d = -d
	if (d > worst) { worst = d; xworst = $1 }
	if (n == 0 || $2 < ymin) ymin = $2
	if (n == 0 || $2 > ymax) ymax = $2
	n++
}
END {
	if (n == 0) { print "no contour was traced at all"; exit }
	if (nseg != 1) printf "%d segments, expected 1\n", nseg
	if (worst > tol) printf "contour traced at x = %.4f, wanted %.4f (off by %.4f)\n", xworst, want, worst
	if (ymin > 0.01 || ymax < 0.99) printf "contour spans only y = %g to %g, expected 0 to 1\n", ymin, ymax
}' decay.txt >> fail
