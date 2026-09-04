#!/usr/bin/env bash
# Minimal, hand-checkable example: three planes all striking due north, with
# dips of 0, 45 and 90, drawn as traces (-W) plus their poles (-S).
# The horizontal plane (dip 0) traces the rim and has its pole at the center;
# the vertical plane (dip 90) traces a straight N-S diameter and has its pole
# on the rim due west; the 45 degree plane sits halfway between the two.
ps=minimal.ps

cat << EOF > minimal.txt
0	0
0	45
0	90
EOF

gmt psstereonet minimal.txt -JA8c -B -W1p,red -Sc0.25c -Gblue -P \
	--MAP_GRID_PEN_PRIMARY=0.25p,gray > $ps
