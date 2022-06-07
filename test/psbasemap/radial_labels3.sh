#!/usr/bin/env bash
# Test 3 of axis labels for radial W/E sides in -JP
gmt begin radial_labels3 pdf,ps
	gmt subplot begin 2x2 -R-30/30/3000/6371 -Fs7c/12c -M8p -B -BWESN -A+jCM -Y2c
		gmt subplot set 0 -A"-JP6c"
		gmt basemap -JP6c  -Bya+l"Depth (km)"
		gmt subplot set 1 -A"-JP6c+z"
		gmt basemap -JP6c+z -Bya+l"Depth (km)"
		gmt subplot set 2 -A"-JP6c+a+t90"
		gmt basemap -JP6c+a+t90 -Bya+l"Depth (km)"
		gmt subplot set 3 -A"-JP6c+z+a+t90"
		gmt basemap -JP6c+z+a+t90 -Bya+l"Depth (km)"
	gmt subplot end
gmt end show
