#!/usr/bin/env bash
#
# Check spiders and geospiders

ps=spiders.ps
gmt set PROJ_ELLIPSOID Sphere

# Cartesian
gmt psbasemap -R0/6/0/3 -Jx1i -P -Xc -Bafg -K > $ps
echo 0.5 0.5 30 100  | gmt psxy -R -J -O -K -Sw2i -Glightyellow -W2p >> $ps
echo 2.5 0.5 30 100  | gmt psxy -R -J -O -K -Sw2i -Gyellow >> $ps
echo 4.5 0.5 30 100  | gmt psxy -R -J -O -K -Sw2i -W1 >> $ps
echo 0.5 1.75 30 100 | gmt psxy -R -J -O -K -Sw2i+a0.25i+p0.5p,red -W1p >> $ps
echo 2.5 1.75 30 100 | gmt psxy -R -J -O -K -Sw2i+r15 -W0.5p -Glightyellow >> $ps
echo 4.5 1.75 30 100 | gmt psxy -R -J -O -K -Sw2i+a0.25i+r15+p0.25p -W1p -Gpink >> $ps
# Geographic
gmt psbasemap -Rg -JG0/25/6i -O -Bafg -K -Y3.5i >> $ps
echo 0 0 30 100  | gmt psxy -R -J -O -K -SW4000k -Glightyellow -W2p >> $ps
echo 0 0 -30 -100  | gmt psxy -R -J -O -K -SW3000n/1000n -Gblue -W2p >> $ps
echo 50 -30 -50 -110  | gmt psxy -R -J -O -K -SW30d+a5d+p0.25p -W1p -Gcyan >> $ps
echo -50 -30 50 110  | gmt psxy -R -J -O -K -SW30d+r30 -W1p -Gorange >> $ps
echo -10 80 -60 240  | gmt psxy -R -J -O -SW20d/5d+a5d+r60 -W1p -Gyellow >> $ps
