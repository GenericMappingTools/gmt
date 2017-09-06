#!/bin/bash
#		GMT EXAMPLE 15
#		$Id$
#
# Purpose:	Gridding and clipping when data are missing
# GMT modules:	blockmedian, gmtconvert, grdclip, grdcontour, grdinfo
# GMT modules:	gmtinfo, nearneighbor, pscoast, psmask, surface
# Unix progs:	rm
#
export GMT_PPID=$$
gmt begin ex15 ps
  gmt convert @ship_15.xyz -bo > ship.b
  region=`gmt info ship.b -I1 -bi3d`
  gmt subplot begin 2x2 -M0.05i -Fs2.9i/3i -LWSne -T"Gridding with missing data"
#   Raw nearest neighbor contouring
    gmt nearneighbor $region -I10m -S40k -Gship.nc ship.b -bi
    gmt grdcontour ship.nc -JM -C250 -A1000 -Gd2i -c2,1
#   Grid via surface but mask out area with no data using coastlines
    gmt blockmedian ship.b -b3d > ship_10m.b
    gmt surface ship_10m.b -Gship.nc -bi
    gmt psmask -I10m ship.b -T -Glightgray -bi3d -c2,2
    gmt grdcontour ship.nc -C250 -L-8000/0 -A1000 -Gd2i
#   Grid via surface but mask out area with no data
    gmt psmask -I10m ship_10m.b -bi3d -c1,1
    gmt grdcontour ship.nc -C250 -A1000 -L-8000/0 -Gd2i
    gmt psmask -C
#   Clip data above sealevel then overlay land
    gmt grdclip ship.nc -Sa-1/NaN -Gship_clipped.nc
    gmt grdcontour ship_clipped.nc -C250 -A1000 -L-8000/0 -Gd2i -c1,2
    gmt pscoast -Ggray -Wthinnest
    gmt grdinfo -C -M ship.nc | gmt psxy -Sa0.15i -Wthick -i11,12
  gmt subplot end
gmt end
rm -f ship.b ship_10m.b ship.nc ship_clipped.nc
