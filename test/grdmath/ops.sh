#!/bin/bash
# $Id$
ps=ops.ps
# Create a unit spike at (0,0)
echo 0 0 1 | gmt xyz2grd -GA.grd -R-2/2/-2/2 -I1 -N0
gmt grdmath A.grd CURV = B.grd
gmt grdmath A.grd D2DX2 = C.grd
gmt grdmath A.grd D2DY2 = D.grd
gmt grdmath A.grd D2DX2 A.grd D2DY2 ADD = E.grd
gmt grdmath A.grd EXTREMA = F.grd

gmt makecpt -Cpolar -T-4/5/1 > t.cpt

gmt grdimage A.grd -Ct.cpt -JX3i -B0 -P -K -Y7.5i > $ps
echo -2 -2 Unit Spike | gmt pstext -R -J -O -K -Dj0.1i -F+jLB+f12p >> $ps
gmt grdimage B.grd -Ct.cpt -J -B0 -O -K -X3.25i >> $ps
echo -2 -2 CURV | gmt pstext -R -J -O -K -Dj0.1i -F+jLB+f12p >> $ps
gmt grdimage C.grd -Ct.cpt -J -B0 -O -K -X-3.25i -Y-3.25i >> $ps
echo -2 -2 D2DX2 | gmt pstext -R -J -O -K -Dj0.1i -F+jLB+f12p >> $ps
gmt grdimage D.grd -Ct.cpt -J -B0 -O -K -X3.25i >> $ps
echo -2 -2 D2DY2 | gmt pstext -R -J -O -K -Dj0.1i -F+jLB+f12p >> $ps
gmt grdimage E.grd -Ct.cpt -J -B0 -O -K -X-3.25i -Y-3.25i >> $ps
echo -2 -2 D2DX2 + D2DY2 | gmt pstext -R -J -O -K -Dj0.1i -F+jLB+f12p >> $ps
gmt grdimage F.grd -Ct.cpt -J -B0 -O -K -X3.25i >> $ps
echo -2 -2 EXTREMA | gmt pstext -R -J -O -K -Dj0.1i -F+jLB+f12p >> $ps
gmt psscale -Ct.cpt -D3.125i/-0.2i/6i/0.2ih -L0.1i -O -K -X-3.25i >> $ps
gmt psxy -R -J -O -T >> $ps
