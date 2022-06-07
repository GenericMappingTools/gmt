#!/usr/bin/env bash
#
# Test basemap time axis for gridline inc < 1 second
gmt begin sub1saxis ps
	gmt basemap -JX18ct/5c -R0/30/-15/1 -BlSrn -Bxa5Sf1g0.2+a60+l"time (s)" -Byg1 -Xc -Y3c
	gmt basemap -BlSrn -Bxa5Sf1g1+a60+l"time (s)" -Byg1 -Y9c
	gmt basemap -BlSrn -Bxa5Sf1g5+a60+l"time (s)" -Byg1 -Y9c
gmt end show
