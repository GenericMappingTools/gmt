#!/usr/bin/env bash
# Test based on Bug #1140.  SST.nc is a compressed version of the original grid

ps=SST.ps
gmt makecpt -T-10/30/2 -Cwysiwyg > sst.cpt
gmt grdimage @SST.nc -Bafg -Rg -JG135/45/90/6i -K -Csst.cpt -X0.75i -Yc -P > $ps
gmt pscoast -R -J -K -O -Wblack >> $ps
gmt psscale -R -J -DJRM+w6i/0.2i -Csst.cpt -B8+l"SST" -O >> $ps
