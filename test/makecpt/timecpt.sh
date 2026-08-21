#!/usr/bin/env bash
# Test makecpt and psscale with calendar time axis as x or y

ps=timecpt.ps
gmt set FORMAT_DATE_MAP "o" FORMAT_TIME_PRIMARY_MAP Abbrev
gmt makecpt -T2017T/2018T/1o -Cjet > t.cpt
gmt psxy -R2017-01-01T/2018-01-01T/10000/40000 @HI_arrivals_2017.txt -JX6iT/3.5i -Ct.cpt -i0,1,0 -Sc0.1c -BWsNe -P -Bxa1Og1o -Byafg -K -Xc -Y1.5i > $ps
gmt psscale -Ct.cpt -DJBC -O -K -R -J -Bxa1Og1o >> $ps
gmt psxy -R10000/40000/2017-01-01T/2018-01-01T @HI_arrivals_2017.txt -JX6i/3.5iT -Ct.cpt -i1,0,0 -Sc0.1c -BWSne+t"Hawaii 2017 Visitor Arrivals" -Bya1Og1o -Bxafg -O -K -Y4.5i >> $ps
gmt psscale -Ct.cpt -DJRM -O -R -J -Bxa1Og1o >> $ps
