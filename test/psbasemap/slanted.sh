#!/usr/bin/env bash
# Test slanted Cartesian x-axis annotations
ps=slanted.ps
gmt psbasemap  -R2000/2020/35/45 -JX15c/8c -Ba1f+a90 -P -K -Xc -Y5c > $ps
gmt psbasemap  -R2000/2020/35/45 -JX15c/8c -Ba1f+a40 -O -Y12c >> $ps
