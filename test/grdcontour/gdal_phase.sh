#!/usr/bin/env bash
#
# Check that -Z+p (phase data) still honors the 360-degree wrap.
#
# grdcontour traces everything with GDAL except -Z+p, which stays on
# gmt_contours().  The reason is gmtsupport_setcontjump(): on every cell it
# unwraps that cell's corner values onto the branch nearest its first corner, so
# a 179/-179 pair of nodes reads as 179/181 and no contour is drawn along the
# wrap.  That decision is made per cell, i.e. the field is multivalued, while a
# GDAL band carries one value per node, so the rule cannot be handed to GDAL.
# If -Z+p ever gets routed through the GDAL path this test is what fires.
#
# The grid is a pure sawtooth, z = wrap(x + 90) in -180/180, so it has a branch
# cut at x = 90 where the value jumps from 179 to -179.  Its contours are single
# meridians, analytically at
#
#	level  -120  -60    0   60  120
#	x       150 -150  -90  -30   30
#
# and there must be NOTHING near the cut: without the wrap handling the cell at
# x = 90 spans -180 to 180 and so crosses every single level, painting a
# spurious contour along that meridian for all of them.
#
# -L-150/150 keeps the -180 level, which genuinely does sit on the cut, out of
# the comparison.

rm -f fail

gmt grdmath -R-180/180/-60/60 -I1 X 90 ADD 180 ADD 360 FMOD 180 SUB = saw.nc
gmt grdcontour saw.nc -C60 -L-150/150 -Z+p -JX10c/4c -Dsaw.txt

$AWK '
function report () {
	if (n == 0) return
	nseg[lev]++
	if (xmax - xmin > 0.01) printf "level %g is not a meridian: x from %g to %g\n", lev, xmin, xmax
	if (ymin > -59.99 || ymax < 59.99) printf "level %g spans only y = %g to %g\n", lev, ymin, ymax
	d = xmin - want[lev]; if (d < 0) d = -d
	if (d > 0.51) printf "level %g is at x = %g, expected %g\n", lev, xmin, want[lev]
	if (xmin > 88 && xmin < 92) printf "level %g put a contour on the branch cut at x = %g\n", lev, xmin
}
BEGIN {	want[-120] = 150; want[-60] = -150; want[0] = -90; want[60] = -30; want[120] = 30 }
/^>/ { report(); n = 0; next }
NF >= 3 {
	lev = $3
	if (n == 0) { xmin = xmax = $1; ymin = ymax = $2 }
	if ($1 < xmin) xmin = $1;  if ($1 > xmax) xmax = $1
	if ($2 < ymin) ymin = $2;  if ($2 > ymax) ymax = $2
	n++
}
END {	report()
	nl = 0
	for (L in nseg) {
		nl++
		if (!(L in want)) printf "unexpected level %s in the dump\n", L
		if (nseg[L] != 1) printf "level %s has %d segments, expected 1\n", L, nseg[L]
	}
	if (nl != 5) printf "%d levels traced, expected 5\n", nl
	for (L in want) if (!(L in nseg)) printf "level %s is missing from the dump\n", L
}' saw.txt >> fail
