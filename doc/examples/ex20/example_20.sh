#!/bin/bash
#		GMT EXAMPLE 20
#		$Id$
#
# Purpose:	Extend GMT to plot custom symbols
# GMT modules:	pscoast, psxy
# Unix progs:	rm
#
# Plot a world-map with volcano symbols of different sizes at hotspot locations
# using table from Muller et al., 1993, Geology.
ps=example_20.ps

gmt pscoast -Rg -JR9i -Bx60 -By30 -B+t"Hotspot Islands and Hot Cities" -Gdarkgreen -Slightblue \
	-Dc -A5000 -K > $ps

gmt psxy -R -J @hotspots.txt -Skvolcano -O -K -Wthinnest -Gred >> $ps

# Overlay a few bullseyes at NY, Cairo, Perth, and Montevideo

cat > cities.txt << END
74W	40.45N	0.5
31.15E	30.03N	0.5
115.49E	31.58S	0.5
56.16W	34.9S	0.5
END

gmt psxy -R -J cities.txt -Skbullseye -O >> $ps

rm -f cities.txt
