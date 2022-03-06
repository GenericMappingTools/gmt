#!/usr/bin/env bash
#
# Demonstrate failure to clip ellipses against periodic TM boundary in y.
# See https://forum.generic-mapping-tools.org/t/how-to-avoid-the-funky-look-of-jt-and-se/1224
ps=tissot_TM.ps
gmt makecpt -T0,1,2,3 -Cred,green,blue > t.cpt
cat << EOF > t.txt
30 -60  0	4000k	Red ellipse crosses the periodic boundary near bottom
100 0   1	4000k	Green ellipse safely along Equator inside the map
230 -40 2	7000k	Blue ellipse crosses the periodic boundary near the top
EOF
gmt psxy -R0/360/-80/80 -JT-15/15/16c -Ba30fg30 -SE- -Ct.cpt -Wthin -P -K t.txt --MAP_ANNOT_OBLIQUE=lon_horizontal --MAP_ANNOT_MIN_SPACING=24p > $ps
gmt psxy -R -J -O -K -Gwhite -Wfaint -Sc18p t.txt >> $ps
gmt pstext -R -J -O t.txt -it0 >> $ps
