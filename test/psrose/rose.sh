#!/usr/bin/env bash
# Test gmt psrose with -Cm and basemap

ps=rose.ps

gmt psrose a.txt -R0/1/0/360 -Bx0g0.2 -Byg60 -B+glightgreen -A5r -Glightblue -W1p -: -Sn -JX4i -P -K -Xc -Cm > $ps
gmt psrose a.txt -R0/1/0/360 -Bx0g0.2 -Byg60 -A5 -Glightblue -W1p -: -Sn -JX4i -O -Y5i -Cm >> $ps

