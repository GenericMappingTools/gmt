#!/bin/bash
#
#	$Id$

ps=subset.ps

# Grid is negative longitudes
gmt grdmath -R-179.2/-176.5/-5/-3 -I0.01 X = t.nc
gmt grdmath -R-179.2/-176.5/-5/-3 -I0.01 X 100 MUL COSD Y 50 MUL SIND MUL = int.nc
gmt makecpt -T-179.2/-176.5/0.01 > t.cpt
gmt grdimage t.nc -Iint.nc -JM6i -P -Ct.cpt -K -B1 -BWSne -Xc -Y0.5i > $ps
# We choose larger area but using positive longitudes
gmt grdimage t.nc -Iint.nc -R180.2/185/-6/-2 -JM6i -O -Ct.cpt -B1 -BWSne  -Y5i >> $ps

