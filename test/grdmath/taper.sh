#!/bin/bash
# $Id$
ps=taper.ps
gmt grdmath -R0/10/0/10 -I0.1 2 0 TAPER = x.nc
gmt grdmath -R0/10/0/10 -I0.1 0 2 TAPER = y.nc
gmt makecpt -Chot -T0/1/0.1 > t.cpt
gmt grdimage x.nc -JX3i -Ct.cpt -B1 -BWSne -P -K -Y7i > $ps
gmt grdcontour x.nc -J -O -K -C0.1 >> $ps
gmt grd2xyz x.nc | awk '{if ($2 == 5) print $0}' > x.txt
gmt psxy -R -J -O -K x.txt -Sc0.03i -Gred >> $ps
gmt grdimage y.nc -J -Ct.cpt -B1 -BWSne -O -K -X3.5i >> $ps
gmt grdcontour y.nc -J -O -K -C0.1 >> $ps
gmt grd2xyz y.nc | awk '{if ($1 == 5) print $0}' > y.txt
gmt psxy -R -J -O -K y.txt -Sc0.03i -Gblue >> $ps
gmt psxy -R0/10/-0.1/1.1 -JX6.5i/2.25i x.txt -i0,2 -Bxafg1 -Byafg0.5 -BWSne -O -K -X-3.5i -Y-3i >> $ps
gmt psxy -R -J x.txt -i0,2 -Sc0.03i -Gred -O -K >> $ps
gmt psxy -R -J y.txt -i1,2 -Bxafg1 -Byafg0.5 -BWSne -O -K -Y-2.75i >> $ps
gmt psxy -R -J y.txt -i1,2 -Sc0.03i -Gblue -O >> $ps
