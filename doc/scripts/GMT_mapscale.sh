#!/bin/bash
#	$Id$
#
ps=GMT_mapscale.ps
gmt psbasemap -R0/40/50/56 -JM5i -Baf -P -K -LjML+c53+w1000k+f+l"Scale at 53\312N" -F+glightcyan+c0+p > $ps
gmt psbasemap -R -J -O -K -LjBR+c53+w1000k+l+f -F+p1p+i+gwhite+c0.1i >> $ps
h=`gmt mapproject -R -J -Wh -Di`
h=`gmt math -Q $h 2 DIV =`
lat=`echo 0 $h | gmt mapproject -R -J -I -Di -o1`
gmt psxy -R -J -O -K -Wfaint -A -N << EOF >> $ps
>
0	$lat
40	$lat
>
0	50
40	50
EOF
gmt psxy -R -J -O -T >> $ps
