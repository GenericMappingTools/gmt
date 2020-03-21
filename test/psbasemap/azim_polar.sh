#!/usr/bin/env bash
# Test new implementation of internal latitude annotations for azimuthal maps
gmt begin azim_polar ps
	gmt set FONT_ANNOT_PRIMARY 7p MAP_TICK_LENGTH_PRIMARY 3p MAP_ANNOT_OFFSET_PRIMARY 3p MAP_POLAR_CAP 75/90 FONT_TITLE 12p MAP_FRAME_WIDTH 3p
	gmt subplot begin 2x2 -Fs8c -BWSNE -Ba30fg30 -M18p -X1.5c
		gmt basemap -R0/360/0/90 -JA0/90/? -c -B+t"North Pole"+i
		gmt basemap -R0/360/10/65 -JA0/90/? -c -B+t"North Annulus"+i90
		gmt basemap -R0/360/-90/0 -JA0/-90/? -c -B+t"South Pole"+i-90
		gmt basemap -R0/360/-65/-10 -JA0/-90/? -c -B+t"South Annulus"+i
	gmt subplot end
gmt end show
