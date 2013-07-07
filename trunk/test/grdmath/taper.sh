#!/bin/sh
# $Id$
ps=taper.ps
grdmath -R0/10/0/10 -I0.1 2 0 TAPER = x.nc
grdmath -R0/10/0/10 -I0.1 0 2 TAPER = y.nc
makecpt -Chot -T0/1/0.1 > t.cpt
grdimage x.nc -JX3i -Ct.cpt -B1 -BWSne -P -K -Y7i > $ps
grdcontour x.nc -J -O -K -C0.1 >> $ps
grd2xyz x.nc | awk '{if ($2 == 5) print $0}' > x.txt
psxy -R -J -O -K x.txt -Sc0.03i -Gred >> $ps
grdimage y.nc -J -Ct.cpt -B1 -BWSne -O -K -X3.5i >> $ps
grdcontour y.nc -J -O -K -C0.1 >> $ps
grd2xyz y.nc | awk '{if ($1 == 5) print $0}' > y.txt
psxy -R -J -O -K y.txt -Sc0.03i -Gblue >> $ps
psxy -R0/10/-0.1/1.1 -JX6.5i/2.25i x.txt -i0,2 -Bxafg1 -Byafg0.5 -BWSne -O -K -X-3.5i -Y-3i >> $ps
psxy -R -J x.txt -i0,2 -Sc0.03i -Gred -O -K >> $ps
psxy -R -J y.txt -i1,2 -Bxafg1 -Byafg0.5 -BWSne -O -K -Y-2.75i >> $ps
psxy -R -J y.txt -i1,2 -Sc0.03i -Gblue -O >> $ps
