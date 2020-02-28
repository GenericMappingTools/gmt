#!/usr/bin/env bash
# Test strike/dip symbol from Jose for consistency across quadrants
# This version is a Mercator plot and azimuths are given.
ps=line_geo.ps
# Must temporarily change GMT_USERDIR to the gallery documentation dir
export GMT_USERDIR=`gmt --show-sharedir`/../doc/rst/source/users-contrib-symbols/geology
echo "0 0  60 30" > q1.txt
echo "0 0 150 30" > q2.txt
echo "0 0 240 30" > q3.txt
echo "0 0 330 30" > q4.txt
gmt psxy -R-0.7/0.7/-0.7/0.7 -JM7c -Skgeo-lineation/5c -P -N -W2p,red -Gred -Bag0.5 -BWSne q3.txt -K > $ps
awk '{printf "@~a@~ = %s@.\n", $3}' q3.txt | gmt pstext -R -J -O -K -F+f10p+cBL -Gwhite -Dj0.2c >> $ps
gmt psxy -R -J -Skgeo-lineation/5c -O -N -W2p,red -Gred -Bag0.5 -BWSne q4.txt -K -Y10c >> $ps
awk '{printf "@~a@~ = %s@.\n", $3}' q4.txt | gmt pstext -R -J -O -K -F+f10p+cBL -Gwhite -Dj0.2c >> $ps
gmt psxy -R -J -Skgeo-lineation/5c -O -N -W2p,red -Gred -Bag0.5 -BWSne q1.txt -K -X9c >> $ps
awk '{printf "@~a@~ = %s@.\n", $3}' q1.txt | gmt pstext -R -J -O -K -F+f10p+cBL -Gwhite -Dj0.2c >> $ps
gmt psxy -R -J -Skgeo-lineation/5c -O -N -W2p,red -Gred -Bag0.5 -BWSne q2.txt -K -Y-10c >> $ps
awk '{printf "@~a@~ = %s@.\n", $3}' q2.txt | gmt pstext -R -J -O -K -F+f10p+cBL -Gwhite -Dj0.2c >> $ps
gmt psxy -R -J -O -T >> $ps
