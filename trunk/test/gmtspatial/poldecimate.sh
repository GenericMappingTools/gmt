#!/bin/sh
#	$Id$
# Testing gmtspatial decimation near poles
#
# Input was made this way.
#gmtmath -T0/2000/1 -o1 0 360 RAND = x
#gmtmath -T0/2000/1 -o1 60 90 RAND = y
#gmtmath -T0/2000/1 -o1 0 100 RAND = z
#paste x y z > polar.txt
ps=poldecimate.ps
DATA="${src:=.}"/polar.txt
# NN averaging
gmtspatial -Aa100k -fg $DATA > results.txt
psxy -R0/360/60/90 -JA0/90/4.5i -P -K -BafgWsne $DATA -Sc0.05i -Ggreen -X1.5i -Y0.75i > $ps
echo "90 60 N = 2000" | pstext -R -J -O -K -N -F+f12+jLM -Dj0.25i >> $ps
psxy -R -J -O -K $DATA -Sc0.05i -BafgWsne -Gdarkseagreen1 -Y5i >> $ps
psxy -R -J -O -K results.txt -Sc0.05i -Gred >> $ps
N=`wc -l results.txt | awk '{printf "%d\n", $1}'`
echo "90 60 N = $N" | pstext -R -J -O -K -N -F+f12+jLM -Dj0.25i >> $ps
psxy -R -J -O -T >> $ps
