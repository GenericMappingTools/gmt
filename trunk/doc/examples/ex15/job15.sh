#!/bin/bash
#		GMT EXAMPLE 15
#		$Id: job15.sh,v 1.12 2011-02-28 00:58:03 remko Exp $
#
# Purpose:	Gridding and clipping when data are missing
# GMT progs:	blockmedian, gmtconvert, grdclip, grdcontour, grdinfo, minmax
# GMT progs:	nearneighbor, pscoast, psmask, pstext, surface
# Unix progs:	awk, echo, rm
#
. ../functions.sh
ps=../example_15.ps
gmtconvert ship.xyz -bo > ship.b
#
region=`minmax ship.b -I1 -bi3`
nearneighbor $region -I10m -S40k -Gship.nc ship.b -bi3
info=`grdinfo -C -M ship.nc`
grdcontour ship.nc -JM3i -P -B2WSne -C250 -A1000 -G2i -K -U"Example 15 in Cookbook" > $ps
#
blockmedian $region -I10m ship.b -bi3 -bo > ship_10m.b
surface $region -I10m ship_10m.b -Gship.nc -bi3
psmask $region -I10m ship.b -J -O -K -T -Glightgray -bi3 -X3.6i >> $ps
grdcontour ship.nc -J -B2WSne -C250 -L-8000/0 -A1000 -G2i -O -K >> $ps
#
psmask $region -I10m ship_10m.b -bi3 -J -B2WSne -O -K -X-3.6i -Y3.75i >> $ps
grdcontour ship.nc -J -C250 -A1000 -L-8000/0 -G2i -O -K >> $ps
psmask -C -O -K >> $ps
#
grdclip ship.nc -Sa-1/NaN -Gship_clipped.nc
grdcontour ship_clipped.nc -J -B2WSne -C250 -A1000 -L-8000/0 -G2i -O -K -X3.6i >> $ps
pscoast $region -J -O -K -Ggray -Wthinnest >> $ps
echo $info | $AWK '{print $12,$13}' | psxy -R -J -O -K -Sa0.15i -Wthick >> $ps
echo "-0.3 3.6 24 0 1 CB Gridding with missing data" | pstext -R0/3/0/4 -Jx1i -O -N >> $ps
rm -f ship.b ship_10m.b ship.nc ship_clipped.nc .gmt*
