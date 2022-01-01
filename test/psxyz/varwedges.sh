#!/usr/bin/env bash
#
# Check wedges

ps=varwedges.ps
gmt set PROJ_ELLIPSOID Sphere

# Cartesian
gmt psbasemap -R0/6/0/3 -Jx1i -p135/35 -P -Xc -Bafg -K > $ps
echo 0.5 0.5  0 2i 30 100 | gmt psxyz -R -J -O -K -p -Sw -Gred -W2p >> $ps
echo 2.5 0.5  0 30 100 | gmt psxyz -R -J -O -K -p -Sw2i -Gred >> $ps
echo 4.5 0.5  0 | gmt psxyz -R -J -O -K -p -Sw2i/30/100 -W1p >> $ps
echo 0.5 1.75 0 30 100 | gmt psxyz -R -J -O -K -p -Sw2i+a+p2p >> $ps
echo 2.5 1.75 0 | gmt psxyz -R -J -O -K -p -Sw2i/30/100+r+p2p >> $ps
echo 4.5 1.75 0 | gmt psxyz -R -J -O -p -Sw2i/30/100+a+p2p -Gred >> $ps
