#!/bin/bash
#	$Id$
#
# Test that contours are oriented correctly
# gmt grdraster 5 -R204/206/19/21 -GBigIsland.nc

ps=contours.ps

color_contour () {
	rm -f contour_*.txt
	gmt grdcontour BigIsland.nc -C1000 -Dcontour_%d.txt -F$1 -S8

	for name in contour_*.txt; do
		# For each contour we compute distance a
		gmt mapproject -Gk $name > contour.d
		L=`gmt info -C contour.d | cut -f8`
		$AWK -v L=$L '{print $1, $2, $4/L}' contour.d | gmt psxy -R -J -O -K -Ccontour.cpt -Sc0.005i >> $ps
	done
}

gmt makecpt -Cseis -T0/1/0.1 -Z > contour.cpt

# The bottom map will have contours oriented so that as you move along
# them (color goes from red to blue) the higher topography will be to
# your right (-Fr).

gmt psbasemap -R204/206/19/21 -JM4i -P -B1 -BWSne -K -X2.5i -Y1.25i > $ps
gmt grdcontour BigIsland.nc -J -C1000 -T+d0.1i/0.02i+l -S8 -O -K --FONT_ANNOT_PRIMARY=9p >> $ps

color_contour r

gmt psscale -Ccontour.cpt -D2i/-0.35i+w4i/0.1i+h+jTC -B1 -O -K >> $ps

# The top map will have contours oriented so that as you move along
# them (color goes from red to blue) the higher topography will be to
# your left (-Fl).

gmt psbasemap -R -J -O -B1 -BWsNe -K -Y4.5i >> $ps
gmt grdcontour BigIsland.nc -J -O -C1000 -T+d0.1i/0.02i+l -S8 -K --FONT_ANNOT_PRIMARY=9p >> $ps

color_contour l

gmt psxy -R -J -O -T >> $ps
