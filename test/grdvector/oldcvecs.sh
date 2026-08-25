#!/usr/bin/env bash
ps=oldcvecs.ps

gmt grdmath -R0/60/0/60 -I10 -r 0.5 Y COSD ADD = r.nc
gmt grdmath -R -I10 -r X = az.nc
gmt makecpt -T0.5/1.5 > t.cpt

gmt grdvector r.nc az.nc -A -Z -Ct.cpt -JX4.5i -Q0.02i/0.15i/0.08i -W1p -S3i -P -K -B30g30 -Xc -Y0.75i > $ps
gmt grd2xyz r.nc | gmt psxy -R -J -O -K -Sc0.05i -Gblack >> $ps
gmt grdvector r.nc az.nc -A -Z -Ct.cpt -J -W1p -S3i -O -K -B30g30 -BWsNE -Y5i >> $ps
gmt grd2xyz r.nc | gmt psxy -R -J -O -Sc0.05i -Gblack >> $ps
