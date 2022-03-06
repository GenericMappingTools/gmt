#!/usr/bin/env bash
gmt begin GMT_stereographic_general
	gmt set GMT_THEME cookbook
	gmt set MAP_ANNOT_OBLIQUE separate
	gmt coast -R100/-42/160/-8+r -JS130/-30/12c -Bag -Dl -A500 -Ggreen -Slightblue -Wthinnest
gmt end show
