#!/usr/bin/env bash
#
# Check wrapping around Greenwich

ps=hexagone.ps

cat > hexagone.dat <<%
-4.5 48.5
-2 43.5
3 42.5
7.5 44
8 49
2.5 51
%

gmt psxy hexagone.dat -R-5/9/42/52 -JM3i -P -L -Gpurple -K > $ps
gmt pscoast -R -J -Dl -Wthin -Ia/thin -N1/thick,red -B2 -O -K --FORMAT_GEO_MAP=D >> $ps

gmt psxy hexagone.dat -R-8.5/-0.5/47/52 -JM3i -Y4i -L -Gpurple -O -K >> $ps
gmt pscoast -R -J -Dl -Wthin -Ia/thin -N1/thick,red -B1 -O -K --FORMAT_GEO_MAP=D >> $ps

gmt psxy hexagone.dat -R1/9/47/52 -JM3i -X4i -L -Gpurple -O -K >> $ps
gmt pscoast -R -J -Dl -Wthin -Ia/thin -N1/thick,red -B1 -O -K --FORMAT_GEO_MAP=D >> $ps

gmt psxy hexagone.dat -R-4/6/42/49.5 -JM3i -Y-4i -L -Gpurple -O -K >> $ps
gmt pscoast -R -J -Dl -Wthin -Ia/thin -N1/thick,red -B1 -O --FORMAT_GEO_MAP=D >> $ps

