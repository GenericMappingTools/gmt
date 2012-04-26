#!/bin/bash
#
#       $Id$

ps=bands.ps

grd2rgb "${src:=.}"/Uluru.ras -Gband_%c.nc 
psimage "$src"/Uluru.ras -W5i -X0.4i -Y4i -Fthicker -K > $ps
echo "0	black	255	red" > t.cpt
grdimage band_r.nc -Ct.cpt -JX5i/0 -O -K -X5.2i -B0 >> $ps
echo "0	black	255	green" > t.cpt
grdimage band_g.nc -Ct.cpt -J -O -K -X-5.2i -Y-3.3i -B0 >> $ps
echo "0	black	255	blue" > t.cpt
grdimage band_b.nc -Ct.cpt -J -O -X5.2i -B0 >> $ps

