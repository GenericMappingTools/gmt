#!/bin/bash
#	$Id$
#
# Evaluate a single (C_l,m, S_l,m) coefficient

ps=one.ps
echo "6	3	0	1" > t.txt
gmt sph2grd t.txt -I1 -Rg -Nm -Ggrid.nc
gmt grd2cpt grid.nc -Cpolar -E7 -T= -Z > t.cpt
gmt grdimage grid.nc -Jx0.015id -P -B30 -BWSnE -Ct.cpt -K -Xc -Y0.75i > $ps
echo "180 90 P@-6,3@-, with S@-6,3@- = 0 and S@-6,3@- = 1" | gmt pstext -R -J -O -K -Dj0.2i -N -F+jBC+f14p >> $ps
echo "6	0	1	0" > t.txt
gmt sph2grd t.txt -I1 -Rg -Nm -Ggrid.nc
gmt grd2cpt grid.nc -Cpolar -E7 -T= -Z > t.cpt
gmt grdimage grid.nc -Jx0.015id -O -K -B30 -BWsnE -Ct.cpt -Y3.25i >> $ps
echo "180 90 P@-6,0@-, with C@-6,0@- = 1 and S@-6,0@- = 0" | gmt pstext -R -J -O -K -Dj0.2i -N -F+jBC+f14p >> $ps
echo "6	6	1	1" > t.txt
gmt sph2grd t.txt -I1 -Rg -Nm -Ggrid.nc
gmt grd2cpt grid.nc -Cpolar -E7 -T= -Z > t.cpt
gmt grdimage grid.nc -Jx0.015id -O -K -B30 -BWsnE -Ct.cpt -Y3.25i >> $ps
echo "180 90 P@-6,6@-, with C@-6,6@- = S@-6,6@- = 1" | gmt pstext -R -J -O -Dj0.2i -N -F+jBC+f14p >> $ps
