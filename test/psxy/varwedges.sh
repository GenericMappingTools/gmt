#!/usr/bin/env bash
#
# Check wedges and geowedges

ps=varwedges.ps
gmt set PROJ_ELLIPSOID Sphere

# Cartesian
gmt psbasemap -R0/6/0/3 -Jx1i -P -Xc -Bafg -K > $ps
echo 0.5 0.5 30 100 | gmt psxy -R -J -O -K -Sw2i -Gred -W2p >> $ps
echo 2.5 0.5 2i 30 100 | gmt psxy -R -J -O -K -Sw -Gred >> $ps
echo 4.5 0.5 | gmt psxy -R -J -O -K -Sw2i/30/100 -W1p >> $ps
echo 0.5 1.75 30 100 | gmt psxy -R -J -O -K -Sw2i+a+p2p >> $ps
echo 2.5 1.75 2i 30 100 | gmt psxy -R -J -O -K -Sw+r+p2p >> $ps
echo 4.5 1.75 | gmt psxy -R -J -O -K -Sw2i/30/100+a+p2p -Gred >> $ps
# Geographic
gmt psbasemap -Rg -JG0/25/6i -O -Bafg -K -Y3.5i >> $ps
echo 0 0 30 100 | gmt psxy -R -J -O -K -SW4000k -Gred -W2p >> $ps
echo 0 0 3000n -30 -100 | gmt psxy -R -J -O -K -SW -Gblue -W2p >> $ps
echo 50 -30 | gmt psxy -R -J -O -K -SW30d/-50/-110+a+p2p -Gcyan >> $ps
echo -50 -30 30d 50 110 | gmt psxy -R -J -O -K -SW+r+p2p -Gorange >> $ps
echo -10 80 | gmt psxy -R -J -O -SW20d/-60/240 -W2p -Gyellow >> $ps
