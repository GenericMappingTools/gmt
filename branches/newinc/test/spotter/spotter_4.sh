#!/bin/bash
#
#       $Id$

ps=spotter_4.ps

# Build a grid
gmt grdmath -R0/30/0/30 -I0.25 X Y MUL = t.nc
# Design a multi-polygon area of interest
cat << EOF > P.txt
>
5	5
15	5
15	20
5	20
5	5
>
20	20
28	20
28	28
20	28
20	20
EOF
# This is original grid outline, for reference
cat << EOF > G.txt
0	0
30	0
30	30
0	30
0	0
EOF
# Rotation without constraining polygons
gmt grdrotater t.nc -e0/53/60 -Grot.nc > box.txt
gmt grdcontour -R-10/80/-5/56 -JM5i -P -B10 -BWSne t.nc -A100 -C25 -K -Xc > $ps
gmt grdcontour -R -J rot.nc -A100 -C25 -O -K -Wa0.75p,red, -Wc0.25p,red >> $ps
gmt psxy -R -J -O -K G.txt -W1p,blue -A >> $ps
gmt psxy -R -J -O -K box.txt -W1p,red >> $ps
echo 0 53 | gmt psxy -R -J -O -K -Sc0.25i -Gred -N -W0.25p >> $ps
echo 0 53 | gmt psxy -R -J -O -K -Sc0.1i -Gblack -N >> $ps
echo 0 53 0.5i -90 -30 | gmt psxy -R -J -O -K -Sm0.15i+e -Gblack -N -W1p >> $ps
# Rotating with constraining polygons
gmt grdrotater t.nc -FP.txt -e0/53/60 -Grot.nc > R.txt
gmt grdcontour -R-10/80/-5/56 -JM5i -B10 -BWSne t.nc -A100 -C25 -O -K -Y5i >> $ps
gmt grdcontour -R -J rot.nc -A100 -C25 -O -K -Wa0.75p,red, -Wc0.25p,red >> $ps
gmt psxy -R -J -O -K G.txt -W1p,blue -A >> $ps
gmt psxy -R -J -O -K box.txt -W1p,red >> $ps
gmt psxy -R -J -O -K P.txt -W1p >> $ps
gmt psxy -R -J -O -K R.txt -W1p >> $ps
echo 0 53 | gmt psxy -R -J -O -K -Sc0.25i -Gred -N -W0.25p >> $ps
echo 0 53 | gmt psxy -R -J -O -K -Sc0.1i -Gblack -N >> $ps
echo 0 53 0.5i -90 -30 | gmt psxy -R -J -O -K -Sm0.15i+e -Gblack -N -W1p >> $ps
gmt psxy -R -J -O -T >> $ps

