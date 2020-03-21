#!/usr/bin/env bash
gmt begin GMT_stereographic_general
	gmt set MAP_ANNOT_OBLIQUE 0
	gmt coast -R100/-42/160/-8+r -JS130/-30/12c -Bag -Dl -A500 -Ggreen -Slightblue -Wthinnest
gmt end show
