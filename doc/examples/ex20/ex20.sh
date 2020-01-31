#!/usr/bin/env bash
#		GMT EXAMPLE 20
#
# Purpose:	Extend GMT to plot custom symbols
# GMT modules:	coast, plot
# Unix progs:	rm, cat
#
# Plot a world-map with volcano symbols of different sizes at hotspot locations
# using table from Muller et al., 1993, Geology.
gmt begin ex20
	gmt set PROJ_LENGTH_UNIT inch
	gmt coast -Rg -JR22c -B -B+t"Hotspot Islands and Hot Cities" -Gdarkgreen -Slightblue -A5000
	gmt plot @hotspots.txt -Skvolcano -Wthinnest -Gred

	# Overlay a few bullseyes at NY, Cairo, Perth, and Montevideo
	cat > cities.txt <<- END
	74W	40.45N	0.5
	31.15E	30.03N	0.5
	115.49E	31.58S	0.5
	56.16W	34.9S	0.5
	END
	gmt plot cities.txt -Sk@bullseye
	rm -f cities.txt
gmt end show
