#!/usr/bin/env bash
gmt begin GMT_-B_log
	gmt set MAP_GRID_PEN_PRIMARY thinnest,.
	gmt basemap -R1/1000/0/1 -JX7.5cl/0.6c -B1f2g3p+l"Axis Label" -BS
	gmt basemap -B1f2g3l+l"Axis Label" -BS -Y2.15c
	gmt basemap -B1f2g3+l"Axis Label" -BS -Y2.15c
gmt end show
