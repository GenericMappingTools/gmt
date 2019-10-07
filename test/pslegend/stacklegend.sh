#!/usr/bin/env bash
#
# Testing gmt pslegend absolute offsets in classic mode
ps=stacklegend.ps
cat << EOF > legend.txt
S 0.1i T 0.07i red  - 0.3i Ship
S 0.1i c 0.07i blue - 0.3i Satellite
EOF
gmt psbasemap -R1/5/1/5 -JX4i -P -Bafg1 -K -Xa1i -Ya1i > $ps
gmt pslegend -R -J -DjBL+w1.2i+o0.25i -F+gwhite+pthicker legend.txt -O -K -Xa1i -Ya1i >> $ps
gmt psbasemap -R2/6/6/10 -J -O -K -Bafg1 -Xa2i -Ya6i >> $ps
gmt pslegend -R -J -DjBL+w1.2i+o0.25i -F+gwhite+pthicker legend.txt -O -Xa2i -Ya6i >> $ps
rm -f legend.txt
