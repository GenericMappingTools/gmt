#!/bin/bash
#		GMT EXAMPLE 24
#		$Id$
#
# Purpose:	Extract subsets of data based on geospatial criteria
# GMT progs:	gmtselect, pscoast, psxy, minmax
# Unix progs:	echo, cat, awk
#
# Highlight oceanic earthquakes within 3000 km of Hobart and > 1000 km from dateline
ps=example_24.ps
echo "147:13 -42:48 3000 Hobart" > point.d
cat << END > dateline.d
> Our proxy for the dateline
180	0
180	-90
END
R=`minmax -I10 oz_quakes.d`
pscoast $R -JM9i -K -Gtan -Sdarkblue -Wthin,white -Dl -A500 -Ba20f10g10WeSn \
	-U"Example 24 in Cookbook" > $ps
psxy -R -J -O -K oz_quakes.d -Sc0.05i -Gred >> $ps
gmtselect oz_quakes.d -L1000k/dateline.d -Nk/s -C3000k/point.d -fg -R -Il \
	| psxy -R -JM -O -K -Sc0.05i -Ggreen >> $ps
$AWK '{print $1, $2, 0, 2*$3, 2*$3}' point.d | psxy -R -J -O -K -SE -Wfat,white >> $ps
$AWK '{print $1, $2, $4}' point.d | pstext -R -J -O -K -F+f14p,Helvetica-Bold,white+jLT \
	-D0.1i/-0.1i >> $ps
psxy -R -J -O -K point.d -Wfat,white -S+0.2i >> $ps
psxy -R -J -O dateline.d -Wfat,white -A >> $ps
rm -f point.d dateline.d
