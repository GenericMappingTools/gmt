#!/bin/bash
# Test radian annotations in basemap and psscale
ps=radians.ps
gmt makecpt -T-pi/pi -Crainbow -Ww > t.ps
gmt math -T-12pi/12pi/200+n T 4 DIV COS 20 MUL 25 ADD = cos.txt

gmt psbasemap -R-12pi/12pi/0/100 -JX6i/7i -P -Bxa4pi -Byaf -K -Xc -Y1.5i > $ps
gmt psxy -R -J -O -K -W2p cos.txt >> $ps
gmt psscale -R -J -DjRM+w4i/0.2i+o0.5i/0 -Ct.cpt -Bxapi -O -K >> $ps
gmt psscale -R -J -DJCB+w5i/0.2i+o0/0.5i -Ct.cpt -Bxapi2 -O >> $ps
