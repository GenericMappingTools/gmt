#!/bin/sh
#	$Id$
# Testing gmtdp on GSHHS high Australia polygon.

. ../functions.sh
header "Test gmtdp for D-P reduction"

ps=reduce.ps
psxy GSHHS_h_Australia.txt -R112/154/-40/-10 -JM5.5i -P -Sc0.01c -Gred -K -B20WSne -Xc > $ps
gmtdp GSHHS_h_Australia.txt -T50k | psxy -R -J -O -K -W0.25p,blue >> $ps
echo 112 -10 T = 50km | pstext -R -J -O -K -Dj0.1i/0.1i -F+jTL+f18p >> $ps
psxy GSHHS_h_Australia.txt -R -J -O -Sc0.01c -Gred -K -B20Wsne -Y4.7i >> $ps
gmtdp GSHHS_h_Australia.txt -T100k | psxy -R -J -O -K -W0.25p,blue >> $ps
echo 112 -10 T = 100km | pstext -R -J -O -K -Dj0.1i/0.1i -F+jTL+f18p >> $ps
psxy -R -J -O -T >> $ps
pscmp
