#!/usr/bin/env bash
# Test new syntax for view window

gmt begin view
	gmt set FONT_TAG 12p
	gmt subplot begin 3x2 -Fs7.5c -Baf -A+gwhite -M12p -T"Adjusting the view window" -Y1c
	# Unrestricted
	gmt subplot set -A"none"
	gmt coast -Rg -JG-74.0/41.5/?+z3000+v180 -Glightblue -Dc
	# 100000 km
	gmt subplot set -A"120@."
	gmt coast -Rg -JG-74.0/41.5/?+z3000+v120 -Glightblue -Dc
	# 10000 km
	gmt subplot set -A"90@."
	gmt coast -Rg -JG-74.0/41.5/?+z3000+v90 -Glightblue -Dc
	# 1000 km
	gmt subplot set -A"60@."
	gmt coast -Rg -JG-74.0/41.5/?+z3000+v60 -Glightblue -Dc
	# 100 km
	gmt subplot set -A"30@."
	gmt coast -Rg -JG-74.0/41.5/?+z3000+v30 -Glightblue -Dl
	# 10 km
	gmt subplot set -A"10@."
	gmt coast -Rg -JG-74.0/41.5/?+z3000+v10 -Glightblue -Di
	gmt subplot end
gmt end show
