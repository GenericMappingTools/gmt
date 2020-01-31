#!/usr/bin/env bash
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

gmt triangulate $pts -R0/10/0/10 -I0.2 -Gpiramide.nc > /dev/null

gmt grdgradient piramide.nc -D -Gaspect.nc

gmt makecpt -Cred,green,blue,yellow -T-45/315/90 -N > pal.cpt

gmt grdimage aspect.nc -JX10c -Cpal.cpt -P -K -B2 -BWSne -Xc > $ps
gmt psscale -Dx11c/5c+w6c/0.6c+jML+e+n -Cpal.cpt -B90+u@. -O -K >> $ps
gmt makecpt -Cjet -T0/1/0.1 > t.cpt
gmt grdimage piramide.nc -J -O -K -Ct.cpt -B2 -BWSne -Y13c >> $ps
gmt psscale -Dx11c/5c+w8c/0.6c+jML -Ct.cpt -O >> $ps
