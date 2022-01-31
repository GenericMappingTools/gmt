#!/usr/bin/env bash
# Purpose:  Compute Cartesian distances from square and triangular polygons
# DVC_TEST
ps=ldist.ps
# Triangle
cat << EOF > p.txt
-1	-1
1	-1
0	1
-1	-1
EOF
gmt grdmath -R-10/10/-10/10 -I0.1 p.txt LDIST = d.nc
gmt grdcontour d.nc -JX4i -P -A5 -C0.25 -Bafg -K -Xc > $ps
gmt psxy -R -J p.txt -W2p,red -O -K >> $ps
# Square
cat << EOF > p.txt
-1	-1
1	-1
1	1
-1	1
-1	-1
EOF
gmt grdmath -R-10/10/-10/10 -I0.1 p.txt LDIST = d.nc
gmt grdcontour d.nc -J -O -A5 -C0.25 -Bafg -K -Y4.75i >> $ps
gmt psxy -R -J p.txt -W2p,red -O >> $ps
