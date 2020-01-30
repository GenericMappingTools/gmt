#!/usr/bin/env bash
gmt set MAP_ANNOT_OBLIQUE 30
gmt coast -R-25/59/70/72+r -JS10/90/11c -B20g -Dl -A250 -Gdarkbrown -Wthinnest -Slightgray -ps GMT_stereographic_rect
