#!/usr/bin/env bash
# Test new syntax for view altitude

gmt begin altitude ps
	gmt set FONT_TAG 12p
	gmt subplot begin 3x2 -Fs7.5c -Baf -A -T"Adjusting the view altitude" -Y1c
	# Infinity
	gmt subplot set -A"@~\245@~"
	gmt coast -Rg -JG-74.0/41.5/? -Ggreen -Dc
	# 100000 km
	gmt subplot set -A"100000"
	gmt coast -Rg -JG-74.0/41.5/?+z100000 -Ggreen -Dc
	# 10000 km
	gmt subplot set -A"10000"
	gmt coast -Rg -JG-74.0/41.5/?+z10000 -Ggreen -Dc
	# 1000 km
	gmt subplot set -A"1000"
	gmt coast -Rg -JG-74.0/41.5/?+z1000 -Ggreen -Dc
	# 100 km
	gmt subplot set -A"100"
	gmt coast -Rg -JG-74.0/41.5/?+z100 -Ggreen -Dl
	# 10 km
	gmt subplot set -A"10"
	gmt coast -Rg -JG-74.0/41.5/?+z10 -Ggreen -Di
	gmt subplot end
gmt end show
