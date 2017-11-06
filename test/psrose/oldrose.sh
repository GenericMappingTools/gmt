#!/bin/bash
# Test gmt psrose with -Cm and basemap with old arrow attribute syntax

ps=oldrose.ps

gmt psrose a.txt -R0/1/0/360 -Bx0g0.2 -Byg60 -B+glightgreen -A5r -Glightblue -W1p -: -Sn -JX4i -P -K -Xc -Cm -M0.3i+e+gred -Wv2p,red --MAP_VECTOR_SHAPE=0.5 > $ps
gmt psrose a.txt -R0/1/0/360 -Bx0g0.2 -Byg60 -A5 -Glightblue -W1p -: -Sn -JX4i -O -Y5i -Cm -M0.1c/0.3i/0.1i/255/0/0 --MAP_VECTOR_SHAPE=0.5 >> $ps
