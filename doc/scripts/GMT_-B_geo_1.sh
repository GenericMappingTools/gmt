#!/bin/sh
#	$Id: GMT_-B_geo_1.sh,v 1.1 2004-07-14 00:46:17 pwessel Exp $
#

gmtset PLOT_DEGREE_FORMAT ddd:mm:ssF
psbasemap -R-1/2/0/0.4 -JM3 -Ba1f15mg5mS -K -P > GMT_-B_geo_1.ps
psxy -R -JM -O -K -Sv0.005/0.02/0.015 -Gblack -Y-0.35i -N -V << EOF >> GMT_-B_geo_1.ps
-0.5 0 0 0.5
-0.5 0 180 0.5
0.375 0 0 0.125
0.375 0 180 0.125
1.29166666 0 0 0.04166666
1.29166666 0 180 0.04166666
EOF
pstext -R -JM -O << EOF >> GMT_-B_geo_1.ps
-0.5 0.05 9 0 0 CB annotation
0.375 0.05 9 0 0 CB frame
1.29166666 0.05 9 0 0 CB grid
EOF
