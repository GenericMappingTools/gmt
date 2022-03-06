#!/usr/bin/env bash
#

ps=part_geo_y.ps

gmt psbasemap -JX12c/8cd -R40/50/-60/20 -Bx2 -By10 -BswNE --FORMAT_GEO_MAP=DG -fi1y -P -K > $ps
gmt psbasemap -J -R -Bx2 -By10 -BswNE --FORMAT_GEO_MAP=DG -fi1x -O -Y12c >> $ps
