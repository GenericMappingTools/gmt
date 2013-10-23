#!/bin/bash
#		GMT EXAMPLE 15
#		$Id$
#
# Purpose:	Gridding and clipping when data are missing
# GMT progs:	blockmedian, gmtconvert, grdclip, grdcontour, grdinfo, gmtinfo
# GMT progs:	nearneighbor, pscoast, psmask, pstext, surface
# Unix progs:	echo, rm
#
ps=example_15.ps
gmt gmtconvert ship.xyz -bo > ship.b
#
region=`gmt info ship.b -I1 -bi3d`
gmt nearneighbor $region -I10m -S40k -Gship.nc ship.b -bi
gmt grdcontour ship.nc -JM3i -P -B2 -BWSne -C250 -A1000 -Gd2i -K > $ps
#
gmt blockmedian $region -I10m ship.b -b3d > ship_10m.b
gmt surface $region -I10m ship_10m.b -Gship.nc -bi
gmt psmask $region -I10m ship.b -J -O -K -T -Glightgray -bi3d -X3.6i >> $ps
gmt grdcontour ship.nc -J -B -C250 -L-8000/0 -A1000 -Gd2i -O -K >> $ps
#
gmt psmask $region -I10m ship_10m.b -bi3d -J -B -O -K -X-3.6i -Y3.75i >> $ps
gmt grdcontour ship.nc -J -C250 -A1000 -L-8000/0 -Gd2i -O -K >> $ps
gmt psmask -C -O -K >> $ps
#
gmt grdclip ship.nc -Sa-1/NaN -Gship_clipped.nc
gmt grdcontour ship_clipped.nc -J -B -C250 -A1000 -L-8000/0 -Gd2i -O -K -X3.6i >> $ps
gmt pscoast $region -J -O -K -Ggray -Wthinnest >> $ps
gmt grdinfo -C -M ship.nc | gmt psxy -R -J -O -K -Sa0.15i -Wthick -i11,12 >> $ps
echo "-0.3 3.6 Gridding with missing data" | gmt pstext -R0/3/0/4 -Jx1i \
	-F+f24p,Helvetica-Bold+jCB -O -N >> $ps
rm -f ship.b ship_10m.b ship.nc ship_clipped.nc
