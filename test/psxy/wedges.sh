#!/usr/bin/env bash
#
# Check wedges and geowedges

ps=wedges.ps
gmt set PROJ_ELLIPSOID Sphere

# Cartesian
gmt psbasemap -R0/6/0/3 -Jx1i -P -Xc -Bafg -K > $ps
echo 0.5 0.5 30 100  | gmt psxy -R -J -O -K -Sw2i -Gred -W2p >> $ps
echo 2.5 0.5 30 100  | gmt psxy -R -J -O -K -Sw2i -Gred >> $ps
echo 4.5 0.5 30 100  | gmt psxy -R -J -O -K -Sw2i -W1p >> $ps
echo 0.5 1.75 30 100 | gmt psxy -R -J -O -K -Sw2i+a+p2p >> $ps
echo 2.5 1.75 30 100 | gmt psxy -R -J -O -K -Sw2i+r+p2p >> $ps
echo 4.5 1.75 30 100 | gmt psxy -R -J -O -K -Sw2i+a+p2p -Gred >> $ps
# Geographic
gmt psbasemap -Rg -JG0/25/6i -O -Bafg -K -Y3.5i >> $ps
echo 0 0 30 100  | gmt psxy -R -J -O -K -SW4000k -Gred -W2p >> $ps
echo 0 0 -30 -100  | gmt psxy -R -J -O -K -SW3000n -Gblue -W2p >> $ps
echo 50 -30 -50 -110  | gmt psxy -R -J -O -K -SW30d+a+p2p -Gcyan >> $ps
echo -50 -30 50 110  | gmt psxy -R -J -O -K -SW30d+r+p2p -Gorange >> $ps
echo -10 80 -60 240  | gmt psxy -R -J -O -SW20d -W2p -Gyellow >> $ps
