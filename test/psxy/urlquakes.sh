#!/usr/bin/env bash
# Test reading directly from URL
# Note: The URL tends to change every few years...
ps=urlquakes.ps

SITE="https://earthquake.usgs.gov/fdsnws/event/1/query.csv"
TIME="starttime=2016-01-01%2000:00:00&endtime=2016-02-01%2000:00:00"
MAG="minmagnitude=4.5"
ORDER="orderby=magnitude"
URL="${SITE}?${TIME}&${MAG}&${ORDER}"

gmt makecpt -Cred,green,blue -T0,70,300,10000 > q.cpt
gmt pscoast -Rg -JN180/7i -Gbisque -Sazure1 -Baf -B+t"Earthquakes for January 2016 with M @~\263@~ 4.5" -Xc -K -P > $ps
gmt psxy -R -J -O -K $URL -hi1 -Sci -Cq.cpt -i2,1,3,4+s0.01 >> $ps
gmt psxy -R -J -O @ridge.txt -W0.5p >> $ps
