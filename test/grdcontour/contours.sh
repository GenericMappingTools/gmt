#!/bin/sh
#	$Id: contours.sh,v 1.2 2010-07-12 15:15:45 remko Exp $
#
# Test that contours are oriented correctly
# grdraster 5 -R204/206/19/21 -GBigIsland.nc

. ../functions.sh
header "Test grdcontour for oriented contours with -Fl/r"

ps=contours.ps
psbasemap -R204/206/19/21 -JM4i -P -B1WSne -K -X2.5i -Y1.25i > $ps
grdcontour BigIsland.nc -J -C1000 -T0.1i/0.02i:-+ -S8 -O -K --ANNOT_FONT_SIZE=9 >> $ps
rm -f contour_*.xyz
# The bottom map will have contours oriented so that as you move along
# them (color goes from red to blue) the higher topography will be to
# your right (-Fr).
grdcontour BigIsland.nc -J -C1000 -Dcontour -Fr -S8 > /dev/null
makecpt -Cseis -T0/1/0.1 -Z > $$.cpt
psscale -C$$.cpt -D2i/-0.35i/4i/0.1ih -B1 -O -K >> $ps
ls contour_*.xyz > $$.lis
while read name; do
	# For each contour we compute distance a
	mapproject -Gk $name > $$.d
	L=`minmax -C $$.d | cut -f8`
	awk '{print $1, $2, $4/'"$L"'}' $$.d | psxy -R -J -O -K -C$$.cpt -Sc0.005i >> $ps
done < $$.lis
# The top map will have contours oriented so that as you move along
# them (color goes from red to blue) the higher topography will be to
# your left (-Fl).
psbasemap -R -J -O -B1WsNe -K -Y4.5i >> $ps
grdcontour BigIsland.nc -J -O -C1000 -T0.1i/0.02i:-+ -S8 -K --ANNOT_FONT_SIZE=9 >> $ps
rm -f contour_*.xyz
grdcontour BigIsland.nc -J -C1000 -Dcontour -Fl -S8 > /dev/null
ls contour_*.xyz > $$.lis
makecpt -Cseis -T0/1/0.1 -Z > $$.cpt
while read name; do
	mapproject -Gk $name > $$.d
	L=`minmax -C $$.d | cut -f8`
	awk '{print $1, $2, $4/'"$L"'}' $$.d | psxy -R -J -O -K -C$$.cpt -Sc0.005i >> $ps
done < $$.lis
psxy -R -J -O /dev/null >> $ps

rm -f contour_*.xyz $$.*

pscmp
