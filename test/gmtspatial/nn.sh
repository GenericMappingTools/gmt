#!/bin/sh
#	$Id$
# Testing gmtspatial nearest neighbor options
#
# Input was made this way.
#gmtmath -T0/9/1 -o1 0 10 RAND = x
#gmtmath -T0/9/1 -o1 0 10 RAND = y
#gmtmath -T0/9/1 -o1 0 100 RAND = z
#paste x y z > points.txt
ps=nn.ps
# NN analysis
gmtspatial -Aa0k -fg points.txt > results.txt
gmtset MAP_FRAME_TYPE plain
psxy -R-0.5/10.5/-0.5/10.5 -JM4.5i -P -Bag1WSne points.txt -Sc0.1i -Gred -K -Xc > $ps
awk '{print $1, $2, NR-1}' points.txt | pstext -R -J -O -K -F+f12+jCB -Dj0.1i >> $ps
rm -f tmp
# Draw links of nearest neighbors
while read x y z w d A B; do
	echo "> $A to $B" >> tmp
	let A=A+1
	let B=B+1
	sed -n ${A}p points.txt >> tmp
	sed -n ${B}p points.txt >> tmp
done < results.txt
psxy -R -J -O -K -W1p,green tmp >> $ps
# NN averaging
gmtspatial -Aa75k -fg points.txt > results.txt
psxy -R -J -O -K -Bag1Wsne points.txt -Sc0.1i -Glightgray -Y4.75i points.txt >> $ps
awk '{print $1, $2, 0, 150, 150}' results.txt | psxy -R -J -O -K -SE -W0.5p,blue >> $ps
psxy -R -J -O -K results.txt -Sc0.1i -Gred >> $ps
psxy -R -J -O -T >> $ps
