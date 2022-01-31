#!/usr/bin/env bash
# Test single hole points in grdmask, including the poles
# DVC_TEST
ps=holes.ps
gmt math -T-90/90/10 -N2/1 0 = points.txt
gmt makecpt -T0/1/1 -Cyellow > t.cpt
gmt grdmask -Ghole.nc -I30m -R-75/75/-90/90 -N1/1/NaN -S4d points.txt
gmt grdimage hole.nc -JQ0/7i -Bxafg180 -Byafg10 -BWSne+t"NaN mask for points with r = 4 degrees" -Ct.cpt -P -X0.75i > $ps
