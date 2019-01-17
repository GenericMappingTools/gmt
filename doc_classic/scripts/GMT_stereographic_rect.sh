#!/usr/bin/env bash
#
gmt set MAP_ANNOT_OBLIQUE 30
gmt pscoast -R-25/59/70/72r -JS10/90/11c -B20g -Dl -A250 -Gdarkbrown -Wthinnest -P \
	-Slightgray > GMT_stereographic_rect.ps
