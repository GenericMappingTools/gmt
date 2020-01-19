#!/usr/bin/env bash
# Plot r,az vectors on the globe
ps=vectors.ps
gmt grdmath -Rg -I30 -r 0.5 Y COSD ADD = r.nc
gmt grdmath -Rg -I30 -r X = az.nc
gmt makecpt -T0.5/1.5 > t.cpt

gmt grdvector r.nc az.nc -A -Z -Ct.cpt -JG45/45/4.5i -Q0.2i+e+h0.5 -W2p+c -Si2500k -P -K -B30g30 -Xc -Y0.75i > $ps
gmt grd2xyz r.nc | gmt psxy -R -J -O -K -SE-200 -Gblack >> $ps
gmt grdvector r.nc az.nc -A -Z -Ct.cpt -J -W2p -Si2500k -O -K -B30g30 -Y5i >> $ps
gmt grd2xyz r.nc | gmt psxy -R -J -O -SE-200 -Gblack >> $ps
#rm -f az.nc r.nc t.cpt
