#!/usr/bin/env bash
# Test gmt psevents with two polygons of different lifetimes

ps=polygons.ps

cat << EOF > polygon.txt
> Polygon -T200/90 -Gpink -W2p,red
0	0
90	-0.5
180	0.9
111	1.2
55	0.8
0	0
> Polygon -T250/60 -Glightblue -W4p
200	0
290	-0.5
380	0.9
311	1.2
255	0.8
200	0
EOF
gmt psevents -R-40/400/-2/2 -JX15c/10c -Baf -B+t"Polygon" polygon.txt -As -T248 -Es+r5+f5 -L -P > $ps
