#!/usr/bin/env bash
# Test gmt psevents with some quakes

ps=events.ps

#gmt convert "https://earthquake.usgs.gov/fdsnws/event/1/query.csv?starttime=2018-01-01%2000:00:00&endtime=2018-12-31%2000:00:00&minmagnitude=5&orderby=time-asc" \
#    -i2,1,3,4+s50,0 -hi1 > quakes_2018.txt
gmt makecpt -Cred,green,blue -T0,70,300,10000 > q.cpt
gmt psevents -Rg -JG200/5/6i -Baf -B+t"Pacific seismicity in 2018 on June 1" @quakes_2018.txt -SE- -Cq.cpt --TIME_UNIT=d -T2018-06-01T -Es+r2+d6 -Ms5+c0.5 -Mi1+c-0.6 -Mt+c0 -P > $ps
