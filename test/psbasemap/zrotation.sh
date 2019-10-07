#!/usr/bin/env bash
# Test script for plain -p<rot> about map center.
# This should place 4 plots that differ by a rotation about the S pole

ps=zrotation.ps
gmt set MAP_FRAME_TYPE plain
gmt psxy -R0/8.3/0/8.3 -Jx1i -P -K -W1p -X0.1i -Y1.35i -B0 -B+t"Rotation about S pole axis" << EOF > $ps
> x midline
4.15	0
4.15	8.3
> Y midline
0	4.15
8.3	4.15
EOF
gmt pstext -R -J -O -K -F+jTL -Dj0.2i << EOF >> $ps
0.00 4.15 rot = 0
4.15 4.15 rot = 45
0.00 8.30 rot = 180
4.15 8.30 rot = 245
EOF
# LL corner is standard, no rotation
gmt pscoast -R0/360/-90/0 -JA0/-90/3.25i -Bg90a360f360 -O -K -Gred -Dc -X0.45i -Y0.45i >> $ps
# LR corner is 45 degrees rotation
gmt pscoast -R -J -Bg90a360f360 -Gred -Dc -X4.15i -O -K -p45+w0/-90 >> $ps
# UL corner is 180 degrees rotation
gmt pscoast -R -J -Bg90a360f360 -Gred -Dc -X-4.15i -Y4.15i -O -K -p180+w0/-90 >> $ps
# UR corner is 245 degrees rotation
gmt pscoast -R -J -Bg90a360f360 -Gred -Dc -X4.15i -O -p245+w0/-90 >> $ps
