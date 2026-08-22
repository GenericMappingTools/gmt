#!/usr/bin/env bash
# Test South Pole longitude annotations in subplots
gmt begin subplot_south_pole ps
	gmt subplot begin 1x2 -Fs10c -BWSNE -Ba30f30g30 -M15p -X1.5c
		gmt basemap -R-180/180/60/90 -JS0/90/? -c -B+t"North"
		gmt basemap -R-180/180/-90/-60 -JS0/-90/? -c -B+t"South"
	gmt subplot end
gmt end show
