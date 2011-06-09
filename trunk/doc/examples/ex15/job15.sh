#!/bin/bash
#		GMT EXAMPLE 15
#		$Id: job15.sh,v 1.14 2011-06-09 16:41:18 remko Exp $
#
# Purpose:	Gridding and clipping when data are missing
# GMT progs:	blockmedian, gmtconvert, grdclip, grdcontour, grdinfo, minmax
# GMT progs:	nearneighbor, pscoast, psmask, pstext, surface
# Unix progs:	echo, rm
#
. ../functions.sh
ps=../example_15.ps
gmtconvert ship.xyz -bo > ship.b
#
region=`minmax ship.b -I1 -bi3`
nearneighbor $region -I10m -S40k -Gship.nc ship.b -bi3
grdcontour ship.nc -JM3i -P -B2WSne -C250 -A1000 -Gd2i -K -U"Example 15 in Cookbook" > $ps
#
blockmedian $region -I10m ship.b -bi3 -bo > ship_10m.b
surface $region -I10m ship_10m.b -Gship.nc -bi3
psmask $region -I10m ship.b -J -O -K -T -Glightgray -bi3 -X3.6i >> $ps
grdcontour ship.nc -J -B -C250 -L-8000/0 -A1000 -Gd2i -O -K >> $ps
#
psmask $region -I10m ship_10m.b -bi3 -J -B -O -K -X-3.6i -Y3.75i >> $ps
grdcontour ship.nc -J -C250 -A1000 -L-8000/0 -Gd2i -O -K >> $ps
psmask -C -O -K >> $ps
#
grdclip ship.nc -Sa-1/NaN -Gship_clipped.nc
grdcontour ship_clipped.nc -J -B -C250 -A1000 -L-8000/0 -Gd2i -O -K -X3.6i >> $ps
pscoast $region -J -O -K -Ggray -Wthinnest >> $ps
grdinfo -C -M ship.nc | psxy -R -J -O -K -Sa0.15i -Wthick -i11,12 >> $ps
echo "-0.3 3.6 Gridding with missing data" | pstext -R0/3/0/4 -Jx1i \
	-F+f24p,Helvetica-Bold+jCB -O -N >> $ps
rm -f ship.b ship_10m.b ship.nc ship_clipped.nc
