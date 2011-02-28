#!/bin/bash
#		GMT EXAMPLE 23
#		$Id: job23.sh,v 1.19 2011-02-28 00:58:03 remko Exp $
#
# Purpose:	Plot distances from Rome and draw shortest paths
# GMT progs:	grdmath, grdcontour, pscoast, psxy, pstext, grdtrack
# Unix progs:	echo, cat, awk
#
. ../functions.sh
ps=../example_23.ps

# Position and name of central point:

lon=12.50
lat=41.99
name="Rome"

# Calculate distances (km) to all points on a global 1x1 grid

grdmath -Rg -I1 $lon $lat SDIST 111.13 MUL = dist.nc

# Location info for 5 other cities + label justification

cat << END > cities.d
105.87	21.02	HANOI		LM
282.95	-12.1	LIMA		LM
178.42	-18.13	SUVA		LM
237.67	47.58	SEATTLE		RM
28.20	-25.75	PRETORIA	LM
END

pscoast -Rg -JH90/9i -Glightgreen -Sblue -U"Example 23 in Cookbook" -A1000 \
	-B0g30:."Distances from $name to the World": -K -Dc -Wthinnest > $ps

grdcontour dist.nc -A1000+v+ukm+kwhite -Glz-/z+ -S8 -C500 -O -K -J \
	-Wathin,white -Wcthinnest,white,- >> $ps

# For each of the cities, plot great circle arc to Rome with psxy

while read clon clat city; do
	(echo $lon $lat; echo $clon $clat) | psxy -R -J -O -K -Wthickest/red >> $ps
done < cities.d

# Plot red squares at cities and plot names:
psxy -R -J -O -K -Ss0.2 -Gred -Wthinnest cities.d >> $ps
$AWK '{print $1, $2, 12, 0, 9, $4, $3}' cities.d | pstext -R -J -O -K -Dj0.15/0 -Gred -N >> $ps
# Place a yellow star at Rome
echo "$lon $lat" | psxy -R -J -O -K -Sa0.2i -Gyellow -Wthin >> $ps

# Sample the distance grid at the cities and use the distance in km for labels

grdtrack -Gdist.nc cities.d \
	| $AWK '{printf "%s %s 12 0 1 CT %d\n", $1, $2, int($NF+0.5)}' \
	| pstext -R -J -O -D0/-0.2i -N -Wwhite,o -C0.02i/0.02i >> $ps

# Clean up after ourselves:

rm -f cities.d dist.nc .gmt*
