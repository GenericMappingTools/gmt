#!/usr/bin/env bash
# Test new +c|C clipping modifiers in coast's -E option
gmt begin coastclip
	gmt subplot begin 2x1 -R-10/10/40/52 -Fs15c/14c -JM15c -Baf
		# Clip on the inside
		gmt basemap -c
		gmt coast -EFR+c
		gmt basemap -B+gdarkgreen
		gmt clip -C
		# Clip on the outside
		gmt basemap -c
		gmt coast -EFR+C
		gmt basemap -B+gorange
		gmt clip -C
	gmt subplot end
gmt end show
