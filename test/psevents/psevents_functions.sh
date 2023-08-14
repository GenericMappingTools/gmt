#!/usr/bin/env bash
# Test the 4 time-functions in psevents for some settings
# Note event duration becomes sum of rise, plateau, decay times = 3
# We run undocumented psevents -/ to get the functions for an event at t = 0.
gmt begin psevents_functions view
	the_args="-Es+rc1+p1+dc1+fc1 -Ms1.5+c0.5 -Mi1+c-0.2 -Mt1+c0 -Mz0.8+c-0.4"
	gmt events -/cc ${the_args}
	# Plot the four curves
	gmt subplot begin 4x1 -R-2/5/-0.5/2.5 -Fs16c/6c -Bxafg10 -Byafg1 -A -T"${the_args}" --FONT_HEADING=16p
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
