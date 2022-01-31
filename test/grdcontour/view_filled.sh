#!/usr/bin/env bash
# Test grdcontour -N with perspective
# DVC_TEST
ps=view_filled.ps

gmt makecpt -T675/975/25 -Cjet > t.cpt
gmt blockmean @Table_5_11.txt -R0/7/0/7 -I1 > mean.xyz
gmt surface mean.xyz -R -I1 -Gdata.nc
# Draw a filled contour map with perspective using grdcontour
gmt grdcontour data.nc -JX4.08i/4.2i -B2f1 -BWSne -Ct.cpt -A+gwhite -N -Gd3i -S4 -P -K -p125/35 -X1.25i > $ps
# Another viewpoint
gmt grdcontour data.nc -JX -B2f1 -BWSne -Ct.cpt -A+gwhite -N -Gd3i -S4 -O -p215/35 -Y4.5i >> $ps
