#!/bin/sh
#	$Id: GMT_-B_geo_2.sh,v 1.1 2004-07-14 00:46:17 pwessel Exp $
#

gmtset PLOT_DEGREE_FORMAT ddd:mm:ssF ANNOT_FONT_SIZE_PRIMARY +9p
psbasemap -R-2/1/0/0.35 -JM4 -Bpa15mf5mg5mwSe -Bs1f30mg15m -K -P --BASEMAP_TYPE=fancy+ \
  --GRID_PEN_PRIMARY=0.25p,black,. --GRID_CROSS_SIZE_SECONDARY=0.1i --FRAME_WIDTH=0.075i --TICK_LENGTH=0.1i > GMT_-B_geo_2.ps
psxy -R -JM -O -K -SvB0.005/0.02/0.015 -Gblack -Y-0.5i -N -V << EOF >> GMT_-B_geo_2.ps
-1.875 0 0 0.33333
-0.45833 0 0 0.11111
0.541666 0 0 0.11111
EOF
pstext -R -JM -O -K -N << EOF >> GMT_-B_geo_2.ps
-2.1 0.025 10 0 0 RM P:
-1.875 0.05 6 0 0 CB annotation
-0.45833 0.05 6 0 0 CB frame
0.541666 0.05 6 0 0 CB grid
EOF
psxy -R -JM -O -K -SvB0.005/0.02/0.015 -Gblack -Y-0.225i -N -V << EOF >> GMT_-B_geo_2.ps
-1.5 0 0 1.33333
-0.25 0 0 0.66666
0.625 0 0 0.33333
EOF
pstext -R -JM -O -N << EOF >> GMT_-B_geo_2.ps
-2.1 0.025 10 0 0 RM S:
-1.5 0.05 9 0 0 CB annotation
-0.25 0.05 9 0 0 CB frame
0.625 0.05 9 0 0 CB grid
EOF
