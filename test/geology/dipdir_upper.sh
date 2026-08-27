#!/usr/bin/env bash
# Plot the same planes as dip direction/dip (-Td) on the upper hemisphere
# (+u) of a Wulff (equal-angle) net, with a coarser azimuth ring (-A).
ps=dipdir_upper.ps

cat << EOF > planes.txt
90	30
180	45
270	60
0	15
30	45
120	48
225	27
350	80
EOF

gmt psstereonet planes.txt -JS8c -Td+u -A45/15 -W1p,red -Sx0.3c -L1p,blue -P \
	--MAP_GRID_PEN_PRIMARY=0.25p,gray > $ps
