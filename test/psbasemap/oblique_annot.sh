#!/usr/bin/env bash
#
# Oblique frames must annotate a gridline where it actually meets the border.
#
# With MAP_ANNOT_OBLIQUE's tick_extend the annotation goes at the tip of the tick that is extended
# along the gridline.  gmtplot_map_tick draws no such tick once the gridline meets the border at
# less than MAP_ANNOT_MIN_ANGLE, but the annotation code used to extend regardless, dividing its
# offset by a vanishing sine and flinging the label far along the border, detached from the crossing
# it belongs to.  Here the 175E meridian meets the N border at about 6 degrees: its annotation
# belongs at x = 3.1 cm and used to be drawn at x = 2.0 cm on a map only 6 cm wide.  See issue #8418.
#
# The positions below were checked against the crossings computed independently with gmt mapproject.

# Pin everything the extracted table depends on, so that it is the annotation placement being tested
gmt set FORMAT_GEO_MAP ddd:mm:ssF MAP_ANNOT_MIN_SPACING 0 \
	MAP_ANNOT_OBLIQUE anywhere,lon_horizontal,lat_horizontal,tick_extend

gmt psbasemap -JOb168/51/172/32.5/6c -R-500/2300/-400/400+uk -BeNsW -Bxa5f1 -Bya5f1 -P > map.ps

# Pull the annotations out of the PostScript as "label border position_along_that_border_in_cm".
# PSL units are 1/1200 inch measured from the lower left map corner, hence the 2.54/1200 scaling.
tr '\r' '\n' < map.ps | awk '
	{ for (i = 3; i <= NF; i++) if ($i == "M") { x = $(i-2); y = $(i-1) } }	# Remember latest moveto
	/\([0-9]+.[EN]\) [a-z][a-z] Z/ {
		for (i = 1; i <= NF; i++) if ($i ~ /^\(/) { lab = $i; just = $(i+1) }
		if      (just == "bc") printf "%s N %.1f\n", lab, x*2.54/1200
		else if (just == "tc") printf "%s S %.1f\n", lab, x*2.54/1200
		else if (just == "mr") printf "%s W %.1f\n", lab, y*2.54/1200
		else if (just == "ml") printf "%s E %.1f\n", lab, y*2.54/1200
	}' | sort > result.txt

printf '%b' '(165\260E) W 0.7\n(170\260E) W 1.4\n(175\260E) N 3.1\n(35\260N) N 5.1\n(40\260N) N 3.9\n(45\260N) N 2.7\n(50\260N) N 1.5\n(55\260N) N 0.3\n(55\260N) W 0.4\n' > answer.txt

diff -q --strip-trailing-cr answer.txt result.txt
