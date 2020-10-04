#!/usr/bin/env bash
# Test gmt psevents with polygons

ps=polygons.ps

# Create a simple polygon, then only plot what should be visible at t = 200 given a duration of 90
cat << EOF > polygon.txt
> Polygon -T200/90
0	0
90	-0.5
180	0.9
111	1.2
55	0.8
0	0
EOF
gmt psevents -R-40/400/-2/2 -JX15c/10c -Baf -B+t"Polygon" polygon.txt -As -T200 -Es+r5+f5 -W2p,red -L90 -P > $ps
