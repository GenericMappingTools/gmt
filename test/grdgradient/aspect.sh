#!/bin/bash
# $Id: $
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

echo   0  red     90 red     > pal.cpt
echo  90  green  180 green  >> pal.cpt
echo 180  blue   270 blue   >> pal.cpt
echo 270  yellow 360 yellow >> pal.cpt

grdimage aspect.nc -JX12c -Cpal.cpt -P -K > $ps

psscale -D12c/3.0c/6c/0.6c -Cpal.cpt -B90 -O >> $ps
