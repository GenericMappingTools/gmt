#!/usr/bin/env bash
#
# Check clipping of multisegment lines crossing over the horizon

ps=clipping2.ps

gmt pscoast -X1i -Y1i -K -P -A10/1 -Di -Wthin -B60 -JG0/-90/7i -R-180/180/-90/-40 --MAP_FRAME_TYPE=plain > $ps
gmt psxy clipline2.xy -O -P -W0.5p,blue -J -R >> $ps

