#!/bin/bash
# Test psrose with -C and basemap
. ../functions.sh

ps=rose.ps
psrose a.txt -R0/1/0/360 -B0g0.2/g60+glightgreen -A5r -Glightblue -W1p -: -S2in -P -K -Xc -C > $ps
psrose a.txt -R0/1/0/360 -B0g0.2/g60 -A5 -Glightblue -W1p -: -S2in -O -Y5i -C >> $ps

pscmp
