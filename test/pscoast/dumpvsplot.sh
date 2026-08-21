#!/usr/bin/env bash
# Plot Poland directly or via a data dump
ps=dumpvsplot.ps

gmt pscoast -EPL -M > t.txt
gmt psxy -R13/26/48/56 -JM4i -P -W0.25p t.txt -Baf -K -Xc > $ps
gmt pscoast -R -J -EPL+p0.25p -Baf -O -Y5i >> $ps
