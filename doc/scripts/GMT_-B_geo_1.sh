#!/usr/bin/env bash
gmt begin GMT_-B_geo_1
	gmt set FORMAT_GEO_MAP ddd:mm:ssF
	gmt basemap -R-1/2/0/0.4 -JM3i -Ba1f15mg5m -BS
	gmt plot -Sv2p+e+a60 -W0.5p -Gblack -Y-0.35i -N << EOF
-0.5 0 0 0.5
-0.5 0 180 0.5
0.375 0 0 0.125
0.375 0 180 0.125
1.29166666 0 0 0.04166666
1.29166666 0 180 0.04166666
EOF
	gmt text -F+f9p+jCB << EOF
-0.5 0.05 annotation
0.375 0.05 frame
1.29166666 0.05 grid
EOF
gmt end show
