#!/usr/bin/env bash
ps=urlquakes.ps

gmt makecpt -Cred,green,blue -T0,70,300,10000 > q.cpt
gmt pscoast -Rg -JN180/7i -Gbisque -Sazure1 -Baf -B+t"Earthquakes for January 2016 with M @~\263@~ 4.5" -Xc -K -P > $ps
gmt psxy -R -J -O -K @urlquakes.csv -hi1 -Sci -Cq.cpt -i2,1,3,4+s0.01 >> $ps
gmt psxy -R -J -O @ridge.txt -W0.5p >> $ps
