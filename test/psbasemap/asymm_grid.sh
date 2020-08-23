#!/usr/bin/env bash
#Check the asymmetrical gridline ticks for negative grid cross sizes
gmt begin asymm_grid ps
	gmt basemap -R-2/2/-2/2 -JM6i -Bafg1 -B+f
	gmt basemap -Bsg10m -Bpg2m --MAP_GRID_CROSS_SIZE_PRIMARY=-4p --MAP_GRID_CROSS_SIZE_SECONDARY=+8p --MAP_GRID_PEN=default,black
gmt end show
