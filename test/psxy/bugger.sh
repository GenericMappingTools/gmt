#!/usr/bin/env bash
# Follows from http://gmt.soest.hawaii.edu/boards/1/topics/7166?r=7178#message-7178
# from Sabin.  Another case where their *.xy file causes trouble at S pole.
ps=bugger.ps
gmt set MAP_FRAME_TYPE plain
gmt psxy -R-180/180/-90/0 -JW115/6i bugger.txt -Gred -B0 -Bwsne -P -K -Xc -Y0.5i > $ps
gmt psxy -R-180/180/-90/0 -JQ115/6i bugger.txt -Gred -B0 -Bwsne -O -K -Y2i >> $ps
gmt psxy -R-180/180/-90/0 -JH115/6i bugger.txt -Gred -B0 -Bwsne -O -K -Y2i >> $ps
gmt psxy -R-180/180/-90/0 -JN115/6i bugger.txt -Gred -B0 -Bwsne -O -K -Y2i >> $ps
gmt psxy -R-180/180/-90/0 -JR115/6i bugger.txt -Gred -B0 -Bwsne -O -Y2i >> $ps
