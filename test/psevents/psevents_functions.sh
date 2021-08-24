#!/usr/bin/env bash
# Test the 4 time-functions in psevents for some settings
# Note event duration becomes sum of rise, plateau, decay times = 3
# We run undocumented psevents -/ to get the functions for an event at t = 0.
gmt begin psevents_functions ps
	gmt events -/ -Es+r1+p1+d1+f1 -Ms1.5+c0.5 -Mi1+c-0.2 -Mt1+c0 -Mz0.8+c-0.4
	# Plot the four curves
	gmt subplot begin 4x1 -R-2/5/-0.5/1.5 -Fs16c/6c -Bxafg10 -Byafg1 -A -T"-Es+r1+p1+d1+f1 -Ms1.5+c0.5 -Mi1+c-0.2 -Mt1+c0 -Mz0.8+c-0.4" --FONT_HEADING=16p
		gmt subplot set 0 -A"size"
		gmt plot psevents_function.txt -i0,1 -W3p,red
		gmt subplot set 1 -A"intensity"
		gmt plot psevents_function.txt -i0,2 -W3p,red
		gmt subplot set 2 -A"transparency"
		gmt plot psevents_function.txt -i0,3 -W3p,red
		gmt subplot set 3 -A"delta z"
		gmt plot psevents_function.txt -i0,4 -W3p,red
	gmt subplot end
gmt end show
