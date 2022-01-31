#!/usr/bin/env bash
#
# Check that gmt grd2cpt estimates correct upper/lower bounds <= z_min && >= z_max
# We try lower and upper, then flip sign on grid and try min and high
# DVC_TEST

ps=paintallzs.ps
echo "6	3	0	1" > t.txt
gmt set MAP_FRAME_TYPE plain
gmt sph2grd t.txt -I1 -Rg -Nm -Ggrid.nc
gmt grdmath grid.nc 0.2 ADD = grid.nc
gmt grd2cpt grid.nc -Cpolar -E7 -Z > t.cpt
gmt grdimage grid.nc -Jx0.015id/0.01id -P -Ct.cpt -B0 -K -X1.5i -Y0.5i > $ps
gmt psscale -DJRM -Ct.cpt -Rgrid.nc -Bxaf -J -O -K >> $ps
gmt grd2cpt grid.nc -Cpolar -E7 -Sl -Z > t.cpt
gmt grdimage grid.nc -J -Ct.cpt -B0 -O -K -Y2i >> $ps
gmt psscale -DJRM -Ct.cpt -R -Bxaf -By+l"-Sl" -J -O -K >> $ps
gmt grd2cpt grid.nc -Cpolar -E7 -Su -Z > t.cpt
gmt grdimage grid.nc -J -Ct.cpt -B0 -O -K -Y2i >> $ps
gmt psscale -DJRM -Ct.cpt -R -Bxaf -By+l"-Su" -J -O -K >> $ps
gmt grdmath grid.nc NEG = grid.nc
gmt grd2cpt grid.nc -Cpolar -E7 -Sm -Z > t.cpt
gmt grdimage grid.nc -J -Ct.cpt -B0 -O -K -Y2i >> $ps
gmt psscale -DJRM -Ct.cpt -R -Bxaf -By+l"-Sm" -J -O -K >> $ps
gmt grd2cpt grid.nc -Cpolar -E7 -Sh -Z > t.cpt
gmt grdimage grid.nc -J -Ct.cpt -B0 -O -K -Y2i >> $ps
gmt psscale -DJRM -Ct.cpt -R -Bxaf -By+l"-Sh" -J -O >> $ps
