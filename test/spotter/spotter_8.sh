#!/bin/bash
# Test multiple outputs from grdrotator
ps=spotter_8.ps
# Just a small test grid to rotate
gmt grdmath -R-5/5/-5/5 -I0.1 -fg X Y HYPOT = in.nc
cat << EOF > r.txt
# Africa absolute motion model of Muller et al, 1993
-31.6   59.3    10.4    -1.89
-44.5   50.9    20.5    -4.36
-43.0   40.3    35.5    -7.91
-41.2   37.7    42.7    -9.65
-40.8   32.8    50.3    -12.09
-41.7   30.1    58.6    -13.89
-40.9   26.4    68.5    -16.23
-39.6   22.3    73.6    -17.80
-38.9   18.0    80.2    -19.98
-40.9   19.0    84.0    -21.53
-41.9   19.4    90.0    -23.31
-41.4   18.9    100.0   -25.35
-39.5   17.7    110.0   -26.71
-39.7   18.7    118.7   -27.37
-37.5   16.7    130.0   -28.52
EOF
gmt makecpt -Cgray -T0/130/10 -Z -I > s.cpt
gmt grdrotater in.nc -Er.txt -Gout_%.1f.nc -Doutline_%.1f.txt
gmt pscoast -R-20/10/-25/10 -JM6i -Glightgray -P -K -Baf -Xc > $ps
gmt makecpt -Crainbow -T0/8/1 -Z > t.cpt
gmt psxy -R -J -O -K outline_*.txt -W1p -Cs.cpt -L >> $ps
gmt grdimage -R -J -Q out_10.4.nc -Ct.cpt -O -K >> $ps
gmt grdimage -R -J -Q out_58.6.nc -Ct.cpt -O -K >> $ps
gmt grdimage -R -J -Q out_130.0.nc -Ct.cpt -O -K >> $ps
gmt psxy -R -J -O -T >> $ps
