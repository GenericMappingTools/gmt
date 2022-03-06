#!/usr/bin/env bash
# Test 1 of axis labels for radial W/E sides in -JP
gmt set PS_COMMENTS true
gmt begin radial_labels1 pdf,ps
	gmt subplot begin 4x1 -R-30/30/4371/6371 -Fs14c/5c -M4p -B -BWESN -A+jCM -Y2c
		gmt subplot set 0 -A"-R-30/30/4371/6371 -JP14c+t90"
		gmt basemap -JP14c+t90  -Bya+l"Depth (E)"
		gmt subplot set 1 -A"-R-30/30/4371/6371 -JP14c+t90+z"
		gmt basemap -JP14c+t90+z -Bya+l"Depth (E)"
		gmt subplot set 2 -A"-R-30/30/4371/6371 -JP14c+a+t180"
		gmt basemap -JP14c+a+t180 -Bya+l"Depth (km)"
		gmt subplot set 3 -A"-R-30/30/4371/6371 -JP14c+z+a+t180"
		gmt basemap -JP14c+z+a+t180 -Bya+l"Depth (km)"
	gmt subplot end
gmt end show
