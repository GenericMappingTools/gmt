#!/bin/bash
#		GMT EXAMPLE 24
#		$Id$
#
# Purpose:	Extract subsets of data based on geospatial criteria
# GMT progs:	gmtselect, pscoast, psxy, gmtinfo
# Unix progs:	echo, cat, awk
#
# Highlight oceanic earthquakes within 3000 km of Hobart and > 1000 km from dateline
ps=example_24.ps
echo "147:13 -42:48 6000 Hobart" > point.txt
cat << END > dateline.txt
> Our proxy for the dateline
180	0
180	-90
END
R=`gmt info -I10 oz_quakes.txt`
gmt pscoast $R -JM9i -K -Gtan -Sdarkblue -Wthin,white -Dl -A500 -Ba20f10g10 -BWeSn > $ps
gmt psxy -R -J -O -K oz_quakes.txt -Sc0.05i -Gred >> $ps
gmt gmtselect oz_quakes.txt -Ldateline.txt+d1000k -Nk/s -Cpoint.txt+d3000k -fg -R -Il \
	| gmt psxy -R -JM -O -K -Sc0.05i -Ggreen >> $ps
gmt psxy point.txt -R -J -O -K -SE- -Wfat,white >> $ps
$AWK '{print $1, $2, $4}' point.txt | gmt pstext -R -J -O -K -F+f14p,Helvetica-Bold,white+jLT \
	-D0.1i/-0.1i >> $ps
gmt psxy -R -J -O -K point.txt -Wfat,white -S+0.2i >> $ps
gmt psxy -R -J -O dateline.txt -Wfat,white -A >> $ps
rm -f point.txt dateline.txt
