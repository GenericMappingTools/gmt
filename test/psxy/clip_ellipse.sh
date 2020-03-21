#!/usr/bin/env bash
#
# Check clipping of ellipses that should wrap in a global gmt projection

ps=clip_ellipse.ps

gmt set MAP_FRAME_TYPE plain

echo 180 0 0 4000 4000 > ellipse.d
gmt psxy ellipse.d -P -K -JM3i -R-180/180/-60/60 -Bg180 -Wthin -SE -Gred -X1i -Y0.75i > $ps
gmt psxy ellipse.d -O -K -JH0/3i -Rg -Bg180 -Wthin -SE -Gred -X3.5i >> $ps
gmt psxy ellipse.d -O -K -JW0/3i -Rg -Bg180 -Wthin -SE -Gred -X-3.5i -Y1.8i >> $ps
gmt psxy ellipse.d -O -K -JI0/3i -Rg -Bg180 -Wthin -SE -Gred -X3.5i >> $ps
gmt psxy ellipse.d -O -K -JN0/3i -Rg -Bg180 -Wthin -SE -Gred -X-3.5i -Y1.8i >> $ps
gmt psxy ellipse.d -O -K -JR0/3i -Rg -Bg180 -Wthin -SE -Gred -X3.5i >> $ps
gmt psxy ellipse.d -O -K -JKf0/3i -Rg -Bg180 -Wthin -SE -Gred -X-3.5i -Y2i >> $ps
gmt psxy ellipse.d -O -K -JKs0/3i -Rg -Bg180 -Wthin -SE -Gred -X3.5i >> $ps
gmt psxy ellipse.d -O -K -JV0/2i -Rg -Bg180 -Wthin -SE -Gred -X-3i -Y1.8i >> $ps
gmt psxy ellipse.d -O -K -JY0/45/3i -Rg -Bg180 -Wthin -SE -Gred -X3i >> $ps
gmt psxy -R -J -O -T >> $ps

