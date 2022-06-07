#!/usr/bin/env bash
# Ensure -JOa works close to a pole
gmt begin JOb_pole
	gmt set MAP_ANNOT_MIN_SPACING 32p MAP_ANNOT_OBLIQUE separate,lon_horizontal,lat_horizontal
	gmt subplot begin 2x2 -Fs8c -M6p  -R-200/200/-200/200+uk
 		gmt basemap -JOa8/82/46/?  -TdjCM+w3c -Ba5fg -c
 		gmt basemap -JOa8/87/46/?  -TdjCM+w3c -Ba5fg -c
 		gmt basemap -JOa8/-82/46/? -TdjCM+w3c -Ba5fg -c
 		gmt basemap -JOa8/-87/46/? -TdjCM+w3c -Ba5fg -c
 	gmt subplot end
 gmt end show
