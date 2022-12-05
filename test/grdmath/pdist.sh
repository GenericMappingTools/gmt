#!/usr/bin/env bash
# Purpose:  Compute Cartesian and Geographic distances from center point
# Prevent a repeat of https://forum.generic-mapping-tools.org/t/potential-bug-in-gmt6-3-dev-grdmath-pdist/3512/2

ps=pdist.ps
echo 0 0 > p.txt
# Cartesian 5 and 1 contours
gmt grdmath -R-10/10/-10/10 -I0.1 p.txt PDIST = c.nc
gmt grdcontour c.nc -JX4i -P -A5 -C1 -Bafg -K -Xc -GlCM/TR > $ps
gmt psxy -R -J p.txt -Sc4p -Gred -O -K >> $ps
# Geographic 500 and 100 km contours
gmt grdmath -R-10/10/-10/10 -I0.1 -fg p.txt LDIST = g.nc
gmt grdcontour g.nc -J -O -A500 -C100 -Bafg -K -Y4.75i -GlCM/TR >> $ps
gmt psxy -R -J p.txt -Sc4p -Gred -O >> $ps
