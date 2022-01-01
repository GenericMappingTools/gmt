#!/usr/bin/env bash
# Test 2 of axis labels for radial W/E sides in -JP
gmt begin radial_labels2 pdf,ps
	gmt subplot begin 4x1 -R150/210/4371/6371 -Fs14c/5c -M4p -B -BWESN -A+jCM -Y2c
		gmt subplot set 0 -A"-R-150/210/4371/6371 -JP14c+t90"
		gmt basemap -JP14c+t90  -Bya+l"Depth (km)"
		gmt subplot set 1 -A"-R-150/210/4371/6371 -JP14c+t90+z"
		gmt basemap -JP14c+t90+z -Bya+l"Depth (km)"
		gmt subplot set 2 -A"-R-150/210/4371/6371 -JP14c+a+t180"
		gmt basemap -JP14c+a+t180 -Bya+l"Depth (km)"
		gmt subplot set 3 -A"-R-150/210/4371/6371 -JP14c+z+a+t180"
		gmt basemap -JP14c+z+a+t180 -Bya+l"Depth (km)"
	gmt subplot end
gmt end show
