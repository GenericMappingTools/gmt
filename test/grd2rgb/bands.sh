#!/bin/bash
#
#       $Id$

ps=bands.ps

gmt grd2rgb "${src:-.}"/Uluru.ras -Gband_%c.nc 
gmt psimage "${src:-.}"/Uluru.ras -Dx0/0+w5i -X0.4i -Y4i -F+pthicker -K > $ps
echo "0	black	255	red" > t.cpt
gmt grdimage band_r.nc -Ct.cpt -JX5i/0 -O -K -X5.2i -B0 >> $ps
echo "0	black	255	green" > t.cpt
gmt grdimage band_g.nc -Ct.cpt -J -O -K -X-5.2i -Y-3.3i -B0 >> $ps
echo "0	black	255	blue" > t.cpt
gmt grdimage band_b.nc -Ct.cpt -J -O -X5.2i -B0 >> $ps

