#!/usr/bin/env bash
gmt begin GMT_stereographic_rect
	gmt set GMT_THEME cookbook
	gmt set MAP_ANNOT_OBLIQUE lon_horizontal,lat_horizontal,tick_extend,tick_normal
	gmt coast -R-25/59/70/72+r -JS10/90/11c -B20g -Dl -A250 -Gdarkbrown -Wthinnest -Slightgray
gmt end show
