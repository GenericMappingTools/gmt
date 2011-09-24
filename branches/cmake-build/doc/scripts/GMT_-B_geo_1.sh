#!/bin/bash
#	$Id$
#
. ./functions.sh
gmtset FORMAT_GEO_MAP ddd:mm:ssF
psbasemap -R-1/2/0/0.4 -JM3i -Ba1f15mg5mS -K -P > GMT_-B_geo_1.ps
psxy -R -J -O -K -Sv0.005/0.02/0.015 -Gblack -Y-0.35i -N << EOF >> GMT_-B_geo_1.ps
-0.5 0 0 0.5
-0.5 0 180 0.5
0.375 0 0 0.125
0.375 0 180 0.125
1.29166666 0 0 0.04166666
1.29166666 0 180 0.04166666
EOF
pstext -R -J -O -F+f9p+jCB << EOF >> GMT_-B_geo_1.ps
-0.5 0.05 annotation
0.375 0.05 frame
1.29166666 0.05 grid
EOF
