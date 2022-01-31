#!/usr/bin/env bash
#
# Test the new -N option in grdcontour and compare to manual grdview+grdcontour
# DVC_TEST
ps=filled_cont.ps

gmt makecpt -T675/975/25 -Cjet > t.cpt
gmt blockmean @Table_5_11.txt -R0/7/0/7 -I1 > mean.xyz
gmt surface mean.xyz -R -I1 -Gdata.nc
# Draw a filled contour map directly via grdcontour
gmt grdcontour data.nc -JX4.08i/4.2i -B2f1 -BWSne -Nt.cpt -C25 -A50+gwhite -Gd3i -S4 -P -K -Xc > $ps
# Compare to the same map done via filled grdview -Qs plus grdcontour separately
gmt grdview data.nc -J -Ct.cpt -S4 -Qs -W0.25p -O -K -Y4.75i >> $ps
gmt grdcontour data.nc -J -B2f1 -BWSne -C25 -A50+gwhite -Gd3i -S4 -O >> $ps
