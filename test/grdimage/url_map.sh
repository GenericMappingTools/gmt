#!/bin/bash
ps=url_map.ps
gmt makecpt -Cglobe -Z > t.cpt
gmt grdimage -Rd -JI15c -Dr http://larryfire.files.wordpress.com/2009/07/untooned_jessicarabbit.jpg -P -K -Xc > $ps
gmt grdimage http://imina.soest.hawaii.edu/pwessel/etopo5m.nc -J -Ct.cpt -B0 -B+t"grdimage via URLs" -O -Y4i >> $ps
