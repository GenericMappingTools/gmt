#!/usr/bin/env bash
# Test polygon wrapping when -R is not global but polygons may be
# Based on issue # 949.  I identified three polygon of 272 that
# failed to plot: These were 31,49,78.  I am keeping the original
# file as well as the loop code that made individual plots in case
# our fix will cause others to fail.

cob=file2.gmt
proj=M6i
ps=badpols.ps
region=-80/120/0/53
gmt convert $cob -Q31 -fg > tmp
area=`gmt info -fg tmp -I0.1`
gmt psxy -R$region -J$proj -Glightblue -P -K -Baf tmp -Xc > $ps
gmt psxy -R -J -W0.25p -O -K tmp >> $ps
gmt pstext -R -J -O -K -F+cTC+jTC+f18p+t"$area" -Dj0/0.2i >> $ps
region=-80/120/0/53
gmt convert $cob -Q49 -fg > tmp
area=`gmt info -fg tmp -I0.1`
gmt psxy -R$region -J -Glightred -O -K -Baf -Y3i tmp >> $ps
gmt psxy -R -J -W0.25p -O -K tmp >> $ps
gmt pstext -R -J -O -K -F+cTC+jTC+f18p+t"$area" -Dj0/0.2i >> $ps
region=-80/120/-72/-20
gmt convert $cob -Q78 -fg > tmp
area=`gmt info -fg tmp -I0.1`
gmt psxy -R$region -J -Glightgreen -P -Baf -O -K -Y3i tmp >> $ps
gmt convert $cob -Q78 -fg | gmt psxy -R -J -W0.25p -O -K >> $ps
gmt pstext -R -J -O -F+cTC+jTC+f18p+t"$area" -Dj0/0.2i >> $ps

# Loop to plot all polygons
#
#gmt math -T0/291/1 -o1 T = t.lis
#while read no; do
#	ps=$no.ps
#	gmt convert $cob -Q$no -fg | gmt psxy -R$region -J$proj -Glightblue -V -P -Baf -V > $ps
#	gmt psconvert $ps -Tf -A5p -Z
#done < t.lis
