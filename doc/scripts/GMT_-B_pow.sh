#!/bin/bash
#	$Id$
#
gmt gmtset MAP_GRID_PEN_PRIMARY thinnest,.
gmt psbasemap -R0/100/0/0.9 -JX3ip0.5/0.25i -Ba3f2g1p+l"Axis Label" -BS -K -P > GMT_-B_pow.ps
gmt psbasemap -R -J -Ba20f10g5+l"Axis Label" -BS -O -Y0.85i >> GMT_-B_pow.ps
