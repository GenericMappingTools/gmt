#!/usr/bin/env bash
# Plot strike/dip planes (right-hand rule, the -Tp default) on a Schmidt
# (equal-area) net.  Each pole (-S) must sit 90 degrees away from every
# point on its own cyclographic trace (-W).
ps=plane.ps

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

gmt psstereonet planes.txt -JA8c -B -W1p,red -Sx0.3c -L1p,blue -P \
	--MAP_GRID_PEN_PRIMARY=0.25p,gray > $ps
