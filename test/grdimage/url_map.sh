#!/usr/bin/env bash
# Actually, netCDF is not supported via the virtual file system so comment out for mow
# DVC_TEST

ps=url_map.ps
gmt grdimage -Rd -JI15c http://larryfire.files.wordpress.com/2009/07/untooned_jessicarabbit.jpg -P -Xc > $ps
#gmt makecpt -Cglobe > t.cpt
#gmt grdimage /vsicurl/http://imina.soest.hawaii.edu/pwessel/etopo5m.nc -J -Ct.cpt -B0 -B+t"grdimage via URLs" -O -Y4i >> $ps
echo "Note: Will fail if gdal is not built with libcurl"
