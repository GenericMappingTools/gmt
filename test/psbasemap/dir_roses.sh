#!/usr/bin/env bash
# Demonstrate alignment/offset of directional stars with label shifts computed in PSL
# We specify a relative large symbol size so we can more easily see it in these small maps.
gmt begin dir_roses
	gmt set MAP_FRAME_TYPE plain MAP_FRAME_PEN faint
	gmt subplot begin 4x3 -R-1/1/-1/1 -JM6c -Fs6c -Sct -Srl
		gmt basemap -TdjTL+w2c+f+lWEST,EAST,SOUTH,NORTH -c
		gmt basemap -TdjTC+w2c+l -c
		gmt basemap -TdjTR+w2c+f+l -c
		gmt basemap -TdjML+w2c+f+l -c
		gmt basemap -TdjMC+w2c+f -c
		gmt basemap -TdjMR+w2c+f+l -c
		gmt basemap -TdjBL+w2c+f+l -c
		gmt basemap -TdjBC+w2c+f+l -c
		gmt basemap -TdjBR+w2c+f+lWEST,EAST,SOUTH,NORTH -c
	gmt subplot end
gmt end show
