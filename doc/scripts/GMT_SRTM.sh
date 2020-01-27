#!/usr/bin/env bash
#	Show distribution of SRTM tiles
gmt begin GMT_SRTM
	gmt set MAP_FRAME_TYPE plain
	gmt coast -R-180/180/-60/60 -JQ0/15c -B -BWStr -Dc -A5000 -Glightgray --FORMAT_GEO_MAP=dddF
	echo "1	red" > t.cpt
	gmt grdmath @srtm_tiles.nc 0 NAN = t.nc
	gmt grdimage t.nc -Ct.cpt
	gmt coast -Dc -A5000 -W0.25p
gmt end show
