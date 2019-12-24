#!/usr/bin/env bash
gmt begin GMT_-B_pow
	gmt set MAP_GRID_PEN_PRIMARY thinnest,.
	gmt basemap -R0/100/0/0.9 -JX3ip0.5/0.25i -Ba3f2g1p+l"Axis Label" -BS
	gmt basemap -Ba20f10g5+l"Axis Label" -BS -Y0.85i
gmt end show
