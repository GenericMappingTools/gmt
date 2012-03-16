#!/bin/bash
#
#	$Id$

header "Test grdimage for 360 offset in header and region"

# Grid is negative longitudes
grdmath -R-179.2/-176.5/-5/-3 -I0.01 X = t.nc
grdmath -R-179.2/-176.5/-5/-3 -I0.01 X 100 MUL COSD Y 50 MUL SIND MUL = int.nc
makecpt -T-179.2/-176.5/0.01 > t.cpt
grdimage t.nc -Iint.nc -JM6i -P -Ct.cpt -K -B1WSne > $ps
# We choose larger area but using positive longitudes
grdimage t.nc -Iint.nc -R180.2/185/-6/-2 -JM6i -O -Ct.cpt -B1WSne  -Y5i >> $ps

pscmp
