#!/usr/bin/env bash
# 
# Purpose: 3-D view of Colorado DEM with shade from large spacecraft over Denver

ps=denver.ps
radius_of_spaceship=100
gmt makecpt -Cjet -T1000/4000 > t.cpt
gmt grdcut @earth_relief_30s -R-108/-103/35/40 -Gtopo.nc
gmt grdgradient topo.nc -A225 -Nt1 -Gint.nc
gmt grdmath -Rint.nc 104.9903W 39.7392N SDIST = r.nc
gmt grdmath r.nc $radius_of_spaceship LT = inside.nc
gmt grdgradient topo.nc -A90 -Nt1 -Gint1.nc
gmt grdgradient topo.nc -A-45 -Nt1 -Gint2.nc
gmt grdmath int1.nc int2.nc 0.5 MUL ADD inside.nc ADD 0.5 SUB = int.nc
gmt grdview topo.nc -Iint.nc -JM5i -JZ0.25i -p155/25 -Ct.cpt -P -K -Baf -Qi200i -X0.75i -N1000+glightgray > $ps
gmt psscale -Ct.cpt -Rint.nc -J -DJCB+w4i/0.2i+h+o0/0.5i -p -O -Baf >> $ps
