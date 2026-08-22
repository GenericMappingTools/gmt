#!/usr/bin/env bash
# Test gmtspatial truncating a polygon to a -R setting that
# is entirely inside the polygon, so should result in the -R
# pollygon outline as answer.  Cartesian case.  Issue #851,a
ps=atruncate.ps
cat << EOF > tpoly.xy
-300 -3500
-200 -800
400 -780
500 -3400
-300 -3500
EOF
gmt spatial tpoly.xy -C -R0/100/-3100/-3000 > clipped.txt

gmt psbasemap -R-500/600/-3600/-700 -JX6i/9i -P -Baf -D0/100/-3100/-3000 -F+p3p,gray -K -Xc > $ps
gmt psxy -R -J tpoly.xy -W1p -O -K >> $ps
gmt psxy -R -J clipped.txt -W0.25p,red -O >> $ps
