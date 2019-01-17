#!/usr/bin/env bash
gmt begin GMT_stereographic_general ps
gmt set MAP_ANNOT_OBLIQUE 0
gmt coast -R100/-42/160/-8r -JS130/-30/4i -Bag -Dl -A500 -Ggreen \
	-Slightblue -Wthinnest 
gmt end
