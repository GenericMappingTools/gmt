#!/bin/sh
#		GMT EXAMPLE 15
#
#		$Id: job15.sh,v 1.2 2003-04-11 23:49:54 pwessel Exp $
#
# Purpose:	Gridding and clipping when data are missing
# GMT progs:	blockmedian, gmtconvert, grdclip, grdcontour, grdinfo, minmax,
#		nearneighbor, pscoast, psmask, pstext, surface
# Unix progs:	awk, echo, rm
#
gmtconvert ship.xyz -bo > ship.b
region=`minmax ship.b -I1 -bi3`
nearneighbor $region -I10m -S40k -Gship.grd ship.b -bi3
grdinfo -C -M ship.grd | cut -f12,13 > tmp
grdcontour ship.grd -JM3i -P -B2WSne -C250 -A1000 -G2i -K -U"Example 15 in Cookbook" > example_15.ps
#
blockmedian $region -I10m ship.b -bi3 -bo > ship_10m.b
surface $region -I10m ship_10m.b -Gship.grd -bi3
psmask $region -I10m ship.b -JM -O -K -T -G220 -bi3 -X3.6i >> example_15.ps
grdcontour ship.grd -JM -B2WSne -C250 -A1000 -L-8000/0 -G2i -O -K >> example_15.ps
#
psmask $region -I10m ship_10m.b -bi3 -JM -B2WSne -O -K -X-3.6i -Y3.75i >> example_15.ps
grdcontour ship.grd -JM -C250 -A1000 -G2i -L-8000/0 -O -K >> example_15.ps
psmask -C -O -K >> example_15.ps
#
grdclip ship.grd -Sa-1/NaN -Gship_clipped.grd
grdcontour ship_clipped.grd -JM -B2WSne -C250 -A1000 -L-8000/0 -G2i -O -K -X3.6i >> example_15.ps
pscoast $region -JM -O -K -G150 -W0.25p >> example_15.ps
psxy tmp -R -JM -O -K -Sa0.15i -W1p >> example_15.ps
echo "-0.3 3.6 24 0 1 CB Gridding with missing data" | pstext -R0/3/0/4 -Jx1i -O -N >> example_15.ps
rm -f ship.b ship_10m.b ship.grd ship_clipped.grd tmp .gmt*
