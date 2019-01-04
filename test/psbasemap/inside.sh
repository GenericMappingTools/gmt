#!/usr/bin/env bash
# Thest inside labeling for Cartesian and geographic maps
ps=inside.ps

gmt psbasemap -R0/13/0/10 -Ba2f0.5 -P -Jm0.4i -K --MAP_FRAME_TYPE=inside -Xc  > $ps
gmt psbasemap -R -Ba2f0.5 -O -Jx0.4i --MAP_FRAME_TYPE=inside  -Y5i >> $ps
