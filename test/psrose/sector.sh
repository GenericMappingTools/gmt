#!/bin/bash
# Test gmt psrose with -Em and basemap amd -Ccpt

ps=sector.ps

gmt makecpt -Chot -T0/3 > t.cpt
gmt psrose a.txt -R0/1/0/360 -Bx0g0.2 -Byg60 -B+glightgreen -A5 -Glightblue -W1p -: -Sn -JX4i -P -K -Xc -Em > $ps
gmt psrose a.txt -R0/1/0/360 -Bx0g0.2 -Byg60 -B+glightgreen -A5 -Ct.cpt -W1p -: -Sn -JX4i -O -Y4.75i -Em >> $ps
