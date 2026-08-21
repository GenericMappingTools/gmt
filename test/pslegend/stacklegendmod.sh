#!/usr/bin/env bash
#
# Testing gmt pslegend absolute offsets in modern mode
cat << EOF > legend.txt
S 0.1i T 0.07i red  - 0.3i Ship
S 0.1i c 0.07i blue - 0.3i Satellite
EOF
gmt begin stacklegendmod ps
  gmt psbasemap -R1/5/1/5 -JX4i -Bafg1 -Xa1i -Ya1i
  gmt pslegend -DjBL+w1.2i+o0.25i -F+gwhite+pthicker legend.txt -Xa1i -Ya1i
  gmt psbasemap -R2/6/6/10 -Bafg1 -Xa2i -Ya6i
  gmt pslegend -DjBL+w1.2i+o0.25i -F+gwhite+pthicker legend.txt -Xa2i -Ya6i
gmt end show
rm -f legend.txt
