#!/bin/bash
# Test gmt psrose with -C and basemap

ps=rose.ps

gmt psrose a.txt -R0/1/0/360 -Bx0g0.2 -Byg60 -B+glightgreen -A5r -Glightblue -W1p -: -S2in -P -K -Xc -C > $ps
gmt psrose a.txt -R0/1/0/360 -Bx0g0.2 -Byg60 -A5 -Glightblue -W1p -: -S2in -O -Y5i -C >> $ps

