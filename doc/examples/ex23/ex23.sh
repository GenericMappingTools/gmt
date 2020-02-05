#!/usr/bin/env bash
#		GMT EXAMPLE 23
#
# Purpose:	Plot distances from Rome and draw shortest paths
# GMT modules:	grdmath, grdcontour, coast, plot, text, grdtrack
# Unix progs:	echo, cat
#
gmt begin ex23
	# Position and name of central point:
	lon=12.50
	lat=41.99
	name="Rome"

	# Calculate distances (km) to all points on a global 1x1 grid
	gmt grdmath -Rg -I1 $lon $lat SDIST = dist.nc

	# Location info for 5 other cities + label justification
	cat <<- END > cities.txt
	105.87	21.02	LM	HANOI
	282.95	-12.1	LM	LIMA
	178.42	-18.13	LM	SUVA
	237.67	47.58	RM	SEATTLE
	28.20	-25.75	LM	PRETORIA
	END
	gmt coast -Rg -JH90/22c -Glightgreen -Sblue -A1000 -Bg30 -B+t"Distances from $name to the World" -Wthinnest

	gmt grdcontour dist.nc -A1000+v+u" km"+fwhite -Glz-/z+ -S8 -C500 -Wathin,white -Wcthinnest,white,-

	# For each of the cities, plot great circle arc to Rome with gmt plot
	gmt plot -Wthickest,red -Fr$lon/$lat cities.txt

	# Plot red squares at cities and plot names:
	gmt plot -Ss0.5c -Gred -Wthinnest cities.txt
	gmt text -Dj10p/0 -F+f12p,Courier-Bold,red+j -N cities.txt
	# Place a yellow star at Rome
	echo "$lon $lat" | gmt plot -Sa0.5c -Gyellow -Wthin

	# Sample the distance grid at the cities and use the distance in integer km for labels
	gmt grdtrack -Gdist.nc cities.txt -o0:2 --FORMAT_FLOAT_OUT=0:%g,1:%g,2:%.0f \
		| gmt text -D0/-12p -N -Gwhite -W -C2p -F+f12p,Helvetica-Bold+jCT

	# Clean up after ourselves:
	rm -f cities.txt dist.nc
gmt end show
