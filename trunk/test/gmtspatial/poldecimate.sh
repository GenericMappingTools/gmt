#!/bin/bash
#	$Id$
# Testing gmt gmtspatial decimation near poles
#
# Input was made this way.
#gmt gmtmath -T0/2000/1 -o1 0 360 RAND = x
#gmt gmtmath -T0/2000/1 -o1 60 90 RAND = y
#gmt gmtmath -T0/2000/1 -o1 0 100 RAND = z
#paste x y z > polar.txt
ps=poldecimate.ps
DATA="${src:-.}"/polar.txt
# NN averaging
gmt gmtspatial -Aa100k -fg $DATA > results.txt
gmt psxy -R0/360/60/90 -JA0/90/4.5i -P -K -Bafg -BWsne $DATA -Sc0.05i -Ggreen -X1.5i -Y0.75i > $ps
echo "90 60 N = 2000" | gmt pstext -R -J -O -K -N -F+f12+jLM -Dj0.25i >> $ps
gmt psxy -R -J -O -K $DATA -Sc0.05i -Bafg -BWsne -Gdarkseagreen1 -Y5i >> $ps
gmt psxy -R -J -O -K results.txt -Sc0.05i -Gred >> $ps
N=`wc -l results.txt | awk '{printf "%d\n", $1}'`
echo "90 60 N = $N" | gmt pstext -R -J -O -K -N -F+f12+jLM -Dj0.25i >> $ps
gmt psxy -R -J -O -T >> $ps
