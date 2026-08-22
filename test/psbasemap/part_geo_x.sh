#!/usr/bin/env bash
#

ps=part_geo_x.ps

gmt psbasemap -JX12cd/8c -R40/50/-100/0 -Bx2 -By10 -BswNE --FORMAT_GEO_MAP=DG -fi0y -P -K > $ps
gmt psbasemap -J -R -Bx2 -By10 -BswNE --FORMAT_GEO_MAP=DG -fi0x -O -Y12c >> $ps
