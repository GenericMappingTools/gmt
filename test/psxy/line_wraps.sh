#/bin/bash
# Isolated part of issue submitted by Nicky Wright on May 25, 2017 at GSA meeting
ps=line_wraps.ps
gmt set MAP_ANNOT_OBLIQUE 2 MAP_FRAME_TYPE plain FORMAT_GEO_MAP dddF
gmt psxy -R110/-79/-25/70r -JN12c -Bafg -Xc -P -W1p,blue subduct.txt -K > $ps
gmt psxy -Rg -JH120W/17c -Bafg -O -K -W1p,blue subduct.txt -X-1i -Y15c >> $ps
gmt psbasemap -R110/-79/-25/70r -JN12c -A | gmt psxy -Rg -JH120W/16c -O -W0.25p,red >> $ps
