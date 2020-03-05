#!/usr/bin/env bash
# Test new implementation of internal latitude annotations for azimuthal maps
gmt begin azim_global ps
	gmt set FONT_ANNOT_PRIMARY 7p MAP_TICK_LENGTH_PRIMARY 3p MAP_ANNOT_OFFSET_PRIMARY 3p FONT_TITLE 12p MAP_FRAME_WIDTH 3p
	gmt subplot begin 3x1 -Fs14c/7c -BWSNE -Ba30fg30 -M12p -X3c -Y1c
		gmt basemap -Rd -JH0/? -c -B+t"Hammer"+i30
		gmt basemap -Rg -JW0/? -c -B+t"Mollweide"+i
		gmt basemap -JI0/? -c -B+t"Sinusoidal"+i
	gmt subplot end
gmt end show
