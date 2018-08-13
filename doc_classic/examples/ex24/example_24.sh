#!/bin/bash
#		GMT EXAMPLE 24
#
# Purpose:	Extract subsets of data based on geospatial criteria
# GMT modules:	gmtselect, pscoast, psxy, gmtinfo
# Unix progs:	echo, cat
#
# Highlight oceanic earthquakes within 3000 km of Hobart and > 1000 km from dateline
ps=example_24.ps
echo "147:13 -42:48 6000" > point.txt
cat << END > dateline.txt
> Our proxy for the dateline
180	0
180	-90
END
R=`gmt info -I10 @oz_quakes_24.txt`
gmt pscoast $R -JM9i -K -Gtan -Sdarkblue -Wthin,white -Dl -A500 -Ba20f10g10 -BWeSn > $ps
gmt psxy -R -J -O -K @oz_quakes_24.txt -Sc0.05i -Gred >> $ps
gmt select @oz_quakes_24.txt -Ldateline.txt+d1000k -Nk/s -Cpoint.txt+d3000k -fg -R -Il \
	| gmt psxy -R -JM -O -K -Sc0.05i -Ggreen >> $ps
gmt psxy point.txt -R -J -O -K -SE- -Wfat,white >> $ps
gmt pstext point.txt -R -J -F+f14p,Helvetica-Bold,white+jLT+tHobart \
	-O -K -D0.1i/-0.1i >> $ps
gmt psxy -R -J -O -K point.txt -Wfat,white -S+0.2i >> $ps
gmt psxy -R -J -O dateline.txt -Wfat,white -A >> $ps
rm -f point.txt dateline.txt
