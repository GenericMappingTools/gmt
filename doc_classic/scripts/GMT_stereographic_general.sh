#!/usr/bin/env bash
#
gmt set MAP_ANNOT_OBLIQUE 0
gmt pscoast -R100/-42/160/-8r -JS130/-30/4i -Bag -Dl -A500 -Ggreen -P \
	-Slightblue -Wthinnest > GMT_stereographic_general.ps
