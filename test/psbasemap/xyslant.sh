#!/usr/bin/env bash
# Test the angled annotation for x and y Cartesian axes
gmt begin xyslant ps
	gmt basemap -R0/1000/10000/20000 -JX15c/9c -Bx100+l"MY X-LABEL"+a30 -Byaf+l"MY Y-LABEL"
	gmt basemap -Bx100+l"MY X-LABEL" -Byaf+l"MY Y-LABEL"+a30 -Y13c
gmt end show
