#!/usr/bin/env bash
# Minimal, hand-checkable example with only two planes.
# A horizontal plane (dip 0) must have its pole exactly at the center of the
# net.  A vertical plane (dip 90) must plot as a straight diameter line
# through the center, with its pole exactly on the rim, 90 degrees away
# from the strike.
ps=minimal.ps

cat << EOF > minimal.txt
0	0
0	90
0	45
EOF

gmt begin minimal png
	gmt psstereonet minimal.txt -JA8c -Sc0.25c -Gblue -W1,red
gmt end
