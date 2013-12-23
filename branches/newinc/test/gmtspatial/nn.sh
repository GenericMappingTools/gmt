#!/bin/bash
#	$Id$
# Testing gmt gmtspatial nearest neighbor options
#
# Input was made this way.
#gmt gmtmath -T0/9/1 -o1 0 10 RAND = x
#gmt gmtmath -T0/9/1 -o1 0 10 RAND = y
#gmt gmtmath -T0/9/1 -o1 0 100 RAND = z
#paste x y z > points.txt
ps=nn.ps
DATA="${src:-.}"/points.txt
# NN analysis
gmt gmtspatial -Aa0k -fg $DATA > results.txt
gmt gmtset MAP_FRAME_TYPE plain
gmt psxy -R-0.5/10.5/-0.5/10.5 -JM4.5i -P -Bag1 -BWSne $DATA -Sc0.1i -Gred -K -Xc > $ps
awk '{print $1, $2, NR-1}' $DATA | gmt pstext -R -J -O -K -F+f12+jCB -Dj0.1i >> $ps
rm -f tmp
# Draw links of nearest neighbors
while read x y z w d A B; do
	echo "> $A to $B" >> tmp
	let A=A+1
	let B=B+1
	sed -n ${A}p $DATA >> tmp
	sed -n ${B}p $DATA >> tmp
done < results.txt
gmt psxy -R -J -O -K -W1p,green tmp >> $ps
# NN averaging
gmt gmtspatial -Aa75k -fg $DATA > results.txt
gmt psxy -R -J -O -K -Bag1 -BWsne points.txt -Sc0.1i -Glightgray -Y4.75i $DATA >> $ps
awk '{print $1, $2, 0, 150, 150}' results.txt | gmt psxy -R -J -O -K -SE -W0.5p,blue >> $ps
gmt psxy -R -J -O -K results.txt -Sc0.1i -Gred >> $ps
gmt psxy -R -J -O -T >> $ps
