#!/bin/bash
ps=grdtxtclip.ps
gmt grdcontour test.grd -R-74.25/-63.75/-44.0/-39.5 \
       -JB-69/-41.75/-40.625/-42.875/6i -Ba1 \
       -C500 -A1000+e+p+ggreen@50 -K -P > $ps
gmt psxy -R -J -O -K -W20p,red << EOF >> $ps
>
-74.25 -44.0
-63.75 -39.5
>
-63.75 -44.0
-74.25 -39.5
EOF
gmt psclip -C -O >> $ps
