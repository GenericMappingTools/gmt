#!/usr/bin/env bash
# Demonstrate alignment/offset of directional stars with label shifts computed in PSL

gmt begin dir_roses
	gmt set MAP_FRAME_TYPE plain MAP_FRAME_PEN faint
	gmt subplot begin 4x3 -R-1/1/-1/1 -JM6c -Fs6c -Sct -Srl
		gmt basemap -TdjTL+w1c+f+lWEST,EAST,SOUTH,NORTH -c
		gmt basemap -TdjTC+w1c+l -c
		gmt basemap -TdjTR+w1c+f+l -c
		gmt basemap -TdjML+w1c+f+l -c
		gmt basemap -TdjMC+w1c+f -c
		gmt basemap -TdjMR+w1c+f+l -c
		gmt basemap -TdjBL+w1c+f+l -c
		gmt basemap -TdjBC+w1c+f+l -c
		gmt basemap -TdjBR+w1c+f+lWEST,EAST,SOUTH,NORTH -c
	gmt subplot end
gmt end show
