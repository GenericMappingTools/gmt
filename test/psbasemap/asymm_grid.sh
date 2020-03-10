#!/usr/bin/env bash
#Check the asymmetrical gridline ticks for negative grid cross sizes
ps=asymm_grid.ps
gmt psbasemap -R-2/2/-2/2 -JM6i -P -Bafg1 -B+f -K > $ps
gmt psbasemap -R -J -P -Bxg1 -Byg10m -O -K --MAP_GRID_CROSS_SIZE=-8p >> $ps
gmt psbasemap -R -J -P -Byg1 -Bxg10m -O -K --MAP_GRID_CROSS_SIZE=-8p >> $ps
gmt psbasemap -R -J -P -Bxg1 -Byg2m -O -K --MAP_GRID_CROSS_SIZE=-4p >> $ps
gmt psbasemap -R -J -P -Byg1 -Bxg2m -O --MAP_GRID_CROSS_SIZE=-4p >> $ps
