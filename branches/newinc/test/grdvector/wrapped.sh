#!/bin/bash
# $Id$
# Test wrapping of geovector's head across periodic boundary
ps=wrapped.ps
L=6000
echo 345	15	60 3000 > t.txt
echo 185	15	45	3000 >> t.txt
# +n used but L is too small, hence only the head test apply
gmt psxy -R0/360/0/30 -JM6i -P -K -Ba -Gred -S=1i+e+n$L -W3p t.txt -Xc > $ps
gmt psxy -R0/360/0/30 -JM6i -O -K -Ba -S=0.5i+e+n$L+pfaint,blue -W3p t.txt -Y1.5i >> $ps
gmt psxy -R0/360/0/30 -JM6i -O -K -Ba -S=1i+n$L -W3p t.txt -Y1.5i >> $ps
gmt psxy -R0/360/0/30 -JM6i -O -K -Ba -Gred -S=0.25i+e -W3p t.txt -Y1.5i >> $ps
gmt psxy -R0/360/0/30 -JM6i -O -K -Ba -S=0.25i+e+pfaint,blue -W3p t.txt -Y1.5i >> $ps
gmt psxy -R0/360/0/30 -JM6i -O -Ba -S=1i -W3p t.txt -Y1.5i >> $ps
