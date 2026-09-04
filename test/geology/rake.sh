#!/usr/bin/env bash
# Plot a single plane (strike 315, dip 30) together with three lineations on
# it, given by their rake (-Tp+r).  Rake 0 and rake 180 must plot exactly at
# the two ends of the plane's cyclographic trace on the rim (at azimuth 315
# and 135), and rake 90 must plot exactly on the trace at the down-dip point
# (azimuth 45, at a distance from center matching a plunge of 30).
ps=rake.ps

cat << EOF > rake.txt
315	30	0
315	30	90
315	30	180
EOF

gmt psstereonet rake.txt -JA10c -B -Tp+r -W1p,green -Sc0.25c -Ggreen -P \
	--MAP_GRID_PEN_PRIMARY=0.25p,gray > $ps
