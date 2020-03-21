#!/usr/bin/env bash
# Follows from http://gmt.soest.hawaii.edu/boards/1/topics/3091
# from Kara.  Another case where their *.gmt file causes trouble at S pole.
ps=kara.ps
gmt set MAP_FRAME_TYPE plain
gmt psxy -R-180/180/-90/0 -JW80/6i kara.gmt -Gred -B0 -Bwsne -P -K -Xc -Y0.5i > $ps
gmt psxy -R-180/180/-90/0 -JQ80/6i kara.gmt -Gred -B0 -Bwsne -O -K -Y2i >> $ps
gmt psxy -R-180/180/-90/0 -JH80/6i kara.gmt -Gred -B0 -Bwsne -O -K -Y2i >> $ps
gmt psxy -R-180/180/-90/0 -JN80/6i kara.gmt -Gred -B0 -Bwsne -O -K -Y2i >> $ps
gmt psxy -R-180/180/-90/0 -JR80/6i kara.gmt -Gred -B0 -Bwsne -O -Y2i >> $ps
