#!/usr/bin/env bash
# Test slanted Cartesian x-axis annotations
ps=slanted.ps
gmt psbasemap  -R2000/2020/35/40 -JX15c/3c -Bxa1f+a90 -Bya1f -P -K -Xc -Y3c > $ps
gmt psbasemap  -R2000/2020/35/40 -JX15c/3c -Bxa1f+a-60 -Bya1f -O -K -Y6.5c >> $ps
gmt psbasemap  -R2000/2020/35/40 -JX15c/3c -Bxa1f+a45 -Bya1f -O -K -Y6.5c >> $ps
gmt psbasemap  -R2000/2020/35/40 -JX15c/3c -Bxa1f+a30 -Bya1f+ap -O -Y6.5c >> $ps
