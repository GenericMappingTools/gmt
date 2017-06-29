#!/bin/bash
#       $Id$
#
# Check wedges and geowedges

ps=wedges.ps
gmt set PROJ_ELLIPSOID Sphere

# Cartesian
gmt psbasemap -R0/6/0/3 -Jx1i -P -Xc -Bafg -K > $ps
echo 0.5 0.5 30 100  | gmt psxy -R -J -O -K -Sw2i -Gred -W2p >> $ps
echo 2.5 0.5 30 100  | gmt psxy -R -J -O -K -Sw2i -Gred >> $ps
echo 4.5 0.5 30 100  | gmt psxy -R -J -O -K -Sw2i -W1 >> $ps
echo 0.5 1.75 30 100 | gmt psxy -R -J -O -K -Sw2i+a -W2p >> $ps
echo 2.5 1.75 30 100 | gmt psxy -R -J -O -K -Sw2i+r -W2p >> $ps
echo 4.5 1.75 30 100 | gmt psxy -R -J -O -K -Sw2i -Gred >> $ps
echo 4.5 1.75 30 100 | gmt psxy -R -J -O -K -Sw2i+a -W2p >> $ps
# Geographic
gmt psbasemap -Rg -JG0/25/6i -O -Bafg -K -Y3.5i >> $ps
echo 0 0 30 100  | gmt psxy -R -J -O -K -SW4000k -Gred -W2p >> $ps
echo 0 0 -30 -100  | gmt psxy -R -J -O -K -SW3000n -Gblue -W2p >> $ps
echo 50 -30 -50 -110  | gmt psxy -R -J -O -K -SW30d -Gcyan >> $ps
echo 50 -30 -50 -110  | gmt psxy -R -J -O -K -SW30d+a -W2p >> $ps
echo -50 -30 50 110  | gmt psxy -R -J -O -K -SW30d -Gorange >> $ps
echo -50 -30 50 110  | gmt psxy -R -J -O -K -SW30d+r -W2p >> $ps
echo -10 80 -60 240  | gmt psxy -R -J -O -K -SW20d -W2p -Gyellow >> $ps
gmt psxy -R -J -O -T >> $ps
