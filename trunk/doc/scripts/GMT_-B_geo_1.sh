#!/bin/bash
#	$Id: GMT_-B_geo_1.sh,v 1.4 2011-02-28 00:58:03 remko Exp $
#
. functions.sh
gmtset PLOT_DEGREE_FORMAT ddd:mm:ssF
psbasemap -R-1/2/0/0.4 -JM3 -Ba1f15mg5mS -K -P > GMT_-B_geo_1.ps
psxy -R -J -O -K -Sv0.005/0.02/0.015 -Gblack -Y-0.35i -N -V << EOF >> GMT_-B_geo_1.ps
-0.5 0 0 0.5
-0.5 0 180 0.5
0.375 0 0 0.125
0.375 0 180 0.125
1.29166666 0 0 0.04166666
1.29166666 0 180 0.04166666
EOF
pstext -R -J -O << EOF >> GMT_-B_geo_1.ps
-0.5 0.05 9 0 0 CB annotation
0.375 0.05 9 0 0 CB frame
1.29166666 0.05 9 0 0 CB grid
EOF
