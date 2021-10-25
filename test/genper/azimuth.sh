#!/usr/bin/env bash
# Test new syntax for view azimuth

gmt begin azimuth
	gmt set FONT_TAG 12p
	gmt subplot begin 3x2 -Fs7.5c -Baf -A+gwhite -M12p -T"Adjusting the view azimuth" -Y1c
	# Unrestricted
	gmt subplot set -A"0@."
	gmt coast -Rg -JG-74.0/41.5/?+z3000 -Gorange -Dc
	# 100000 km
	gmt subplot set -A"30@."
	gmt coast -Rg -JG-74.0/41.5/?+z3000+a30 -Gorange -Dc
	# 10000 km
	gmt subplot set -A"90@."
	gmt coast -Rg -JG-74.0/41.5/?+z3000+a90 -Gorange -Dc
	# 1000 km
	gmt subplot set -A"180@."
	gmt coast -Rg -JG-74.0/41.5/?+z3000+a180 -Gorange -Dc
	# 100 km
	gmt subplot set -A"225@."
	gmt coast -Rg -JG-74.0/41.5/?+z3000+a225 -Gorange -Dl
	# 10 km
	gmt subplot set -A"-30@."
	gmt coast -Rg -JG-74.0/41.5/?+z3000+a-30 -Gorange -Di
	gmt subplot end
gmt end show
