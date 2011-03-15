#!/bin/bash
#	$Id: contours.sh,v 1.3 2011-03-15 02:06:45 guru Exp $
#
# Test that contours are oriented correctly
# grdraster 5 -R204/206/19/21 -GBigIsland.nc

. ../functions.sh
header "Test grdcontour for oriented contours with -Fl/r"

color_contour () {
	rm -f contour_*.txt
	grdcontour BigIsland.nc -C1000 -Dcontour_%d.txt -F$1 -S8

	for name in contour_*.txt; do
		# For each contour we compute distance a
		mapproject -Gk $name > contour.d
		L=`minmax -C contour.d | cut -f8`
		awk -v L=$L '{print $1, $2, $4/L}' contour.d | psxy -R -J -O -K -Ccontour.cpt -Sc0.005i >> $ps
	done
}

ps=contours.ps
makecpt -Cseis -T0/1/0.1 -Z > contour.cpt

# The bottom map will have contours oriented so that as you move along
# them (color goes from red to blue) the higher topography will be to
# your right (-Fr).

psbasemap -R204/206/19/21 -JM4i -P -B1WSne -K -X2.5i -Y1.25i > $ps
grdcontour BigIsland.nc -J -C1000 -T0.1i/0.02i:-+ -S8 -O -K --FONT_ANNOT_PRIMARY=9p >> $ps

color_contour r

psscale -Ccontour.cpt -D2i/-0.35i/4i/0.1ih -B1 -O -K >> $ps

# The top map will have contours oriented so that as you move along
# them (color goes from red to blue) the higher topography will be to
# your left (-Fl).

psbasemap -R -J -O -B1WsNe -K -Y4.5i >> $ps
grdcontour BigIsland.nc -J -O -C1000 -T0.1i/0.02i:-+ -S8 -K --FONT_ANNOT_PRIMARY=9p >> $ps

color_contour l

psxy -R -J -O /dev/null >> $ps

rm -f contour_*.txt contour.d contour.cpt

pscmp
