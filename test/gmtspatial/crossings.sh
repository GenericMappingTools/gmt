#!/usr/bin/env bash
# Test simple line intersection in gmtspatial
# Should be a single intersection at -45.1337990939  -2.32614829776
ps=crossings.ps

gmt project -C22/49 -E-60/-20 -G10 -Q > line1.dat
gmt project -C0/-60 -E-60/30  -G10 -Q > line2.dat
gmt spatial line1.dat line2.dat -Ie -Fl > intersections.dat
echo "-45.1337990939  -2.32614829776" > answer.txt
n=$(gmt info -Fi -o2 intersections.dat)
gmt pscoast -N1 -R-65/25/-62/65 -JQ22/6i -P -A5000 -Wthinnest -Slightblue -Glightbrown -Bafg -B+t"Number of intersections: $n" -Dl -K -Xc > $ps
gmt psxy -R -J -O -K -Sc0.1i -Ggreen answer.txt >> $ps
gmt psxy line1.dat -R -J -O -Wthick,blue -K >> $ps
gmt psxy line2.dat -R -J -O -Wthick,red -K  >> $ps
gmt psxy -R -J -O -Sc0.1i -W0.5p intersections.dat >> $ps
