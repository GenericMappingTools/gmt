#!/usr/bin/env bash
# Demonstrate alignment/offset of magnetic compasses with label shifts computed in PSL
# While an automatic size could be set, since we use tiny subplots here we select
# a larger size for visibility (here 2.5cm ~ 1 inch).

gmt begin mag_roses
	gmt set MAP_FRAME_TYPE plain MAP_FRAME_PEN faint
	gmt subplot begin 4x3 -R-1/0.99/-1/1 -JM6c -Fs6c -Sct -Srl
		gmt basemap -TmjTL+w2.5c+lWEST,EAST,SOUTH,NORTH -c
		gmt basemap -TmjTC+w2.5c+l+d10 -c
		gmt basemap -TmjTR+w2.5c+l -c
		gmt basemap -TmjML+w2.5c+l -c
		gmt basemap -TmjMC+w4c+d0 -c
		gmt basemap -TmjMR+w2.5c+l -c
		gmt basemap -TmjBL+w2.5c+l -c
		gmt basemap -TmjBC+w2.5c+l+d10 -c
		gmt basemap -TmjBR+w2.5c+lWEST,EAST,SOUTH,NORTH -c
	gmt subplot end
gmt end show
