#!/usr/bin/env bash
# Examine issue #1201
ps=arrowline.ps

cat << EOF > p.txt
0.2 0 0 2
0.4 0 0 1
0.6 0 0 0.5
0.8 0 0 0.25
EOF
# Bottom row
gmt psxy -Jx10c -R0.1/0.9/-0.1/0.7 -SV1c+e+h0 -W0.5p,black -P -X2.5c -Y1.5c -K p.txt -Bx0 -Byg0.1 --PROJ_LENGTH_UNIT=cm > $ps
gmt pstext -R -J -O -K -F+f18p+cTL+tCM -Dj0.1i >> $ps
gmt psxy -J -R -SV1c+e+h0 -W0.5p,black -O -K p.txt -Bx0 -Byg0.1 -X8.5c --PROJ_LENGTH_UNIT=inch >> $ps
gmt pstext -R -J -O -K -F+f18p+cTL+tINCH -Dj0.1i >> $ps
# Middle row
gmt psxy -J -R -SV1c+e+h0+n1c -W0.5p,black -O -K -X-8.5c -Y8.5c p.txt -Bx0 -Byg0.1 --PROJ_LENGTH_UNIT=cm >> $ps
gmt pstext -R -J -O -K -F+f18p+cTL+tCM -Dj0.1i >> $ps
gmt psxy -J -R -SV1c+e+h0+n2c -W0.5p,black -O -K p.txt -Bx0 -Byg0.1 -X8.5c --PROJ_LENGTH_UNIT=inch >> $ps
gmt pstext -R -J -O -K -F+f18p+cTL+tINCH -Dj0.1i >> $ps
# Top row
gmt psxy -J -R -SV1c+e+h0+n1c -W0.5p,black -Gblack -O -K -X-8.5c -Y8.5c p.txt -Bx0 -Byg0.1 --PROJ_LENGTH_UNIT=cm >> $ps
gmt pstext -R -J -O -K -F+f18p+cTL+tCM -Dj0.1i >> $ps
gmt psxy -J -R -SV1c+e+h0+n1i -W0.5p,black -Gblack -O -K p.txt -Bx0 -Byg0.1 -X8.5c --PROJ_LENGTH_UNIT=inch >> $ps
gmt pstext -R -J -O -F+f18p+cTL+tINCH -Dj0.1i >> $ps
