#!/usr/bin/env bash
gmt begin GMT_-B_log
	gmt set MAP_GRID_PEN_PRIMARY thinnest,.
	gmt basemap -R1/1000/0/1 -JX3il/0.25i -B1f2g3p+l"Axis Label" -BS
	gmt basemap -B1f2g3l+l"Axis Label" -BS -Y0.85i
	gmt basemap -B1f2g3+l"Axis Label" -BS -Y0.85i
gmt end show
