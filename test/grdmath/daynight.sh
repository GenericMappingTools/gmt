#!/usr/bin/env bash
# Test the DAYNIGHT operator
# GRAPHICSMAGICK_RMS = 0.004

ps=daynight.ps
gmt grdmath -Rd -I1 30 20 0 DAYNIGHT = a.grd
gmt grdmath -Rd -I1 30 20 5 DAYNIGHT = b.grd
echo "0 black 1 white" > t.cpt
gmt grdimage a.grd -Ct.cpt -Baf -B+t"Transition = 0" -P -K -JQ6.5i > $ps
echo 30 20 | gmt psxy -R -J -O -K -Sk@sunglasses/1.5c >> $ps
gmt grdimage b.grd -Ct.cpt -Baf -B+t"Transition = 5" -J -O -K -Y5i >> $ps
echo 30 20 | gmt psxy -R -J -O -Sc0.25c -Sk@sunglasses/1.5c >> $ps
