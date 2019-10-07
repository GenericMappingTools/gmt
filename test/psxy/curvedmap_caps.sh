#!/usr/bin/env bash
# Testing a S polar cap using -JH for three different central medirian views
# This currently fails so we have a blank page as original
ps=curvedmap_caps.ps
gmt psxy -Rd -JH120E/6i pol_S.txt -Gred -P -K -Y0.75i > $ps
gmt psxy -R -J pol_N.txt -Gblue -B0g60 -O -K >> $ps
gmt psxy -Rd -JH0/6i pol_S.txt -Gred -O -K -Y3.25i >> $ps
gmt psxy -R -J pol_N.txt -Gblue -B0g60 -O -K >> $ps
gmt psxy -Rd -JH120W/6i pol_S.txt -Gred -O -K -Y3.25i >> $ps
gmt psxy -R -J pol_N.txt -Gblue -B0g60 -O >> $ps
