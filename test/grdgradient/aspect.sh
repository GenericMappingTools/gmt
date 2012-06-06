#!/bin/bash
# $Id$
# Test the generation of aspect maps

ps=aspect.ps

pts=pts.dat

echo  0    0   0  > $pts
echo  0   10   0 >> $pts
echo 10   10   0 >> $pts
echo 10    0   0 >> $pts
echo  2.5  2.5 0 >> $pts
echo  2.5  7.5 0 >> $pts
echo  7.5  7.5 0 >> $pts
echo  7.5  2.5 0 >> $pts
echo  5    5   1 >> $pts

triangulate $pts -R0/10/0/10 -I0.2 -Gpiramide.nc > /dev/null

grdgradient piramide.nc -D -Gaspect.nc

echo   -45  red     45 red     > pal.cpt
echo  45  green  135 green  >> pal.cpt
echo 135  blue   225 blue   >> pal.cpt
echo 225  yellow 315 yellow >> pal.cpt

grdimage aspect.nc -JX10c -Cpal.cpt -P -K -B2WSne -Xc > $ps
psscale -D11c/5c/6c/0.6c -Cpal.cpt -B90:,-\\232: -O -K -E+n >> $ps
makecpt -Cjet -T0/1/0.1 > t.cpt
grdimage piramide.nc -J -O -K -Ct.cpt -B2WSne -Y13c >> $ps
psscale -D11c/5c/8c/0.6c -Ct.cpt -O >> $ps
