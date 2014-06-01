#!/bin/bash
#	$Id: subset.sh 12115 2013-09-03 23:22:48Z fwobbe $
# Testing gmt grdcut -Zn

ps=shrink.ps

gmt grdmath -R0/10/0/10 -I1 X Y MUL = t.nc
cat << EOF > replace.txt
0 1 NaN
0 2 NaN
0 3 NaN
3 10 NaN
4 10 NaN
8 9 NaN
8 10 NaN
9 10 NaN
10 10 NaN
7 4 NaN
EOF
gmt grdedit t.nc -Nreplace.txt
gmt makecpt -Cjet -T0/100/5 > t.cpt
gmt psscale -Ct.cpt -A -D0/4.2i/8.4i/0.2i -Ba10 -P -K > $ps
gmt grdimage t.nc -Ct.cpt -JX4i -R -Baf -O -K -X1i >> $ps
echo 10 5 Original | gmt pstext -R -J -O -K -N -Dj0.5i/0 -F+f24p+jLM >> $ps
gmt grdcut t.nc -Zn -Gnew.nc
gmt grdimage new.nc -Ct.cpt -JX4i -R -Baf -BWsNe+t"Strip off NaNs along grid perimeter" -O -K -Y4.4i >> $ps
echo 10 5 -Zn | gmt pstext -R -J -O -N -Dj0.5i/0 -F+f24p+jLM >> $ps
