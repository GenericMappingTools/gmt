#!/bin/bash
#	$Id$
#
gmt gmtset MAP_GRID_PEN_PRIMARY thinnest,.
gmt psbasemap -R1/1000/0/1 -JX3il/0.25i -B1f2g3p+l"Axis Label" -BS -K -P > GMT_-B_log.ps
gmt psbasemap -R -J -B1f2g3l+l"Axis Label" -BS -O -K -Y0.85i >> GMT_-B_log.ps
gmt psbasemap -R -J -B1f2g3+l"Axis Label" -BS -O -Y0.85i >> GMT_-B_log.ps
