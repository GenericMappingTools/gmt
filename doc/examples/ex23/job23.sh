#!/bin/sh
#
#	GMT Example 23  $Id: job23.sh,v 1.2 2004-04-26 22:40:32 pwessel Exp $
#
# Purpose:	Plot distances from Rome and draw shortest paths
# GMT progs:	gmtset, grdmath, grdcontour, psxy, pstext, grdtrack
# Unix progs:	echo, cat, awk

# Position and name of central point:

lon=12.50
lat=41.99
name="Rome"

# Calculate distances (km) to all points on a global 1x1 grid

grdmath -Rg -I1 $lon $lat SDIST 111.13 MUL = dist.grd

# Location info for 5 other cities

cat << EOF > cities.d
139.75	35.67	TOKYO
282.95	-12.1	LIMA
151.17	-33.92	SYDNEY
237.67	47.58	SEATTLE
28.03	-26.17	JOHANNESBURG
EOF

pscoast -Rg -JH90/9i -Glightgreen -Sblue -U"Example 23 in Cookbook" -A1000 \
  -B0g30:."Distances from $name to the World": -K -Dc -Wthinnest > example_23.ps

# Temporarily switch basemap pen in order to annotate white contours

gmtset BASEMAP_FRAME_RGB white
grdcontour dist.grd -A1000t -C500 -O -K -J -Wathin,white -Wcthinnest,- >> example_23.ps
gmtset BASEMAP_FRAME_RGB black

# For each of the cities, plot great circle arc to Rome with psxy

while read clon clat city; do
	(echo $lon $lat; echo $clon $clat) | psxy -R -J -O -K -W2p/red >> example_23.ps
done < cities.d

# Plot red squares at cities and plot names:
psxy -R -J -O -K -Ss0.2 -Gred -W0.25p cities.d >> example_23.ps
awk '{print $1, $2, 12, 1, 9, "LM", $3}' cities.d | pstext -R -J -O -K -D0.15/0 -Gred -N >> example_23.ps
# Place a yellow star at Rome
echo "$lon $lat" | psxy -R -J -O -K -Sa0.2i -Gyellow -Wthin >> example_23.ps

# Sample the distance grid at the cities and use the distance in km for labels

grdtrack -Gdist.grd cities.d \
	| awk '{printf "%s %s 12 0 1 CB %d\n", $1, $2, int($NF+0.5)}' \
	| pstext -R -J -O -D0/0.2i -N -Wwhiteo -C0.02i/0.02i >> example_23.ps

# Clean up after ourselves:

rm -f cities.d dist.grd .gmt*
