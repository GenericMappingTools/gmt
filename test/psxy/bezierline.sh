#!/bin/bash
# Test pen modifier +b for Bezier spline in psxy
ps=bezierline.ps
# Linear
gmt psxy -R-1/4/-1/3 -Jx1i -P -K -Baf -W0.25p -Gred -Sc0.2i t.txt -Xc > $ps
gmt psxy -R -J -O -K -W1p t.txt >> $ps
# Bezier spline
gmt psxy -R -J -O -K -Baf -W0.25p -Gred -Sc0.2i t.txt -Y4.75i >> $ps
gmt psxy -R -J -O -W1p+b t.txt >> $ps
