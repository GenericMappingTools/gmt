#!/usr/bin/env bash
# Test gmtspatial truncating a polygon to a -R setting that
# is tangent to the polygon at W/S.  Fails when polygon exceeds
# x = 190 which is odd since not geographic (?). Issue #851, b
ps=btruncate.ps
cat << EOF > tpoly.xy
0 0
200 0
200 200
0 200
0 0
EOF
gmt spatial tpoly.xy -C -R0/10/0/10 > clipped.txt

gmt psbasemap -R-5/205/-5/205 -JX6i -P -Baf -D0/10/0/10 -F+p3p,gray -K -Xc > $ps
gmt psxy -R -J tpoly.xy -W1p -O -K >> $ps
gmt psxy -R -J clipped.txt -W0.25p,red -O >> $ps
