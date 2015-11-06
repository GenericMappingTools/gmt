#!/bin/bash
#	$Id$
ps=stairs.ps
# Original x-y curve
gmt psxy -R1990T/2017T/-50/100 -JX6iT/2.75i -P -Bxa5Yf1Y -Byafg1000 -BWsNe -W1p stairs.txt -K -X1.25i -Y7i > $ps
gmt psxy -R -J -O -K stairs.txt -Sc0.15c -Gred >> $ps
# Stairs in x
gmt psxy -R -J -O -K -Bxa5Yf1Y -Byafg1000 -BWsnE -W1p stairs.txt -Ax -Y-3i >> $ps
gmt psxy -R -J -O -K stairs.txt -Sc0.15c -Gred >> $ps
# Stairs in y
gmt psxy -R -J -O -K -Bxa5Yf1Y -Byafg1000 -BWSnE -W1p stairs.txt -Ay -Y-3i >> $ps
gmt psxy -R -J -O stairs.txt -Sc0.15c -Gred >> $ps
