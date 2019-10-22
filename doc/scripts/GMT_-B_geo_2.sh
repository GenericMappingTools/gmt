#!/usr/bin/env bash
gmt begin GMT_-B_geo_2
	gmt set FORMAT_GEO_MAP ddd:mm:ssF FONT_ANNOT_PRIMARY +9p
	gmt basemap -R-2/1/0/0.35 -JM4i -Bpa15mf5mg5m -BwSe -Bs1f30mg15m --MAP_FRAME_TYPE=fancy+ \
		--MAP_GRID_PEN_PRIMARY=thinnest,black,. --MAP_GRID_CROSS_SIZE_SECONDARY=0.1i \
		--MAP_FRAME_WIDTH=0.075i --MAP_TICK_LENGTH_PRIMARY=0.1i
	gmt plot -Sv0.03i+b+e+jc -W0.5p -Gblack -Y-0.5i -N << EOF
-1.875 0 0 0.33333
-0.45833 0 0 0.11111
0.541666 0 0 0.11111
EOF
	gmt text -N -F+f+j << EOF
-2.1 0.025 10p RM P:
-1.875 0.05 6p CB annotation
-0.45833 0.05 6p CB frame
0.541666 0.05 6p CB grid
EOF
	gmt plot -Sv0.03i+b+e+jc -W0.5p -Gblack -Y-0.225i -N << EOF
-1.5 0 0 1.33333
-0.25 0 0 0.66666
0.625 0 0 0.33333
EOF
	gmt text -N -F+f+j << EOF
-2.1 0.025 10p RM S:
-1.5  0.05 9p CB annotation
-0.25 0.05 9p CB frame
0.625 0.05 9p CB grid
EOF
gmt end show
