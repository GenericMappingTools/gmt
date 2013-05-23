#!/bin/bash
#	$Id$
#
# Check that gmt grd2cpt estimates correct upper/lower bounds <= z_min && >= z_max

ps=paintallzs.ps
echo "6	3	0	1" > t.txt
gmt sph2grd t.txt -I1 -Rg -Nm -Ggrid.nc
gmt grd2cpt grid.nc -Cpolar -E7 -T+ -Z > t.cpt
gmt grdimage grid.nc -Jx0.015id -P -Ct.cpt -K -Xc -Y0.75i > $ps
