#!/bin/bash
#	$Id: GMT_-B_geo_2.sh,v 1.6 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

gmtset FORMAT_GEO_MAP ddd:mm:ssF FONT_ANNOT_PRIMARY +9p
psbasemap -R-2/1/0/0.35 -JM4i -Bpa15mf5mg5mwSe -Bs1f30mg15m -K -P --MAP_FRAME_TYPE=fancy+ \
	--MAP_GRID_PEN_PRIMARY=thinnest,black,. --MAP_GRID_CROSS_SIZE_SECONDARY=0.1i \
	--MAP_FRAME_WIDTH=0.075i --MAP_TICK_LENGTH=0.1i > GMT_-B_geo_2.ps
psxy -R -J -O -K -SvB0.005i/0.02i/0.015i -Gblack -Y-0.5i -N -V << EOF >> GMT_-B_geo_2.ps
-1.875 0 0 0.33333
-0.45833 0 0 0.11111
0.541666 0 0 0.11111
EOF
pstext -R -J -O -K -N -F+f+j << EOF >> GMT_-B_geo_2.ps
-2.1 0.025 10p RM P:
-1.875 0.05 6p CB annotation
-0.45833 0.05 6p CB frame
0.541666 0.05 6p CB grid
EOF
psxy -R -J -O -K -SvB0.005i/0.02i/0.015i -Gblack -Y-0.225i -N -V << EOF >> GMT_-B_geo_2.ps
-1.5 0 0 1.33333
-0.25 0 0 0.66666
0.625 0 0 0.33333
EOF
pstext -R -J -O -N -F+f+j << EOF >> GMT_-B_geo_2.ps
-2.1 0.025 10p RM S:
-1.5  0.05 9p CB annotation
-0.25 0.05 9p CB frame
0.625 0.05 9p CB grid
EOF
