#!/bin/bash
#	$Id$
#	Show distribution of SRTM tiles
gmt set MAP_FRAME_TYPE plain
gmt pscoast -R-180/180/-60/60 -JQ0/6i -Baf -BWStr -Dc -A5000 -Glightgray -P -K --FORMAT_GEO_MAP=dddF > GMT_SRTM.ps
echo "1	red" > t.cpt
gmt grdmath @srtm_tiles.nc 0 NAN = t.nc
gmt grdimage t.nc -R -J -O -K -Ct.cpt >> GMT_SRTM.ps
gmt pscoast -R -J -Dc -A5000 -W0.25p -O >> GMT_SRTM.ps
