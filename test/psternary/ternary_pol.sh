#!/usr/bin/env bash
# Test ternary diagram plotting with a,b,c-coordinate polygons
ps=ternary_pol.ps
cat << EOF > p.txt
> -Gred
1 0 0
0.5	0 0.5
0.5	0.5 0
1 0 0
> -Gblue
0 0.2 0.8
0.2 0.2 0.6
0.2 0.7 0.1
0 0.7 0.3
0 0.2 0.8
EOF
gmt psternary p.txt -R0/100/0/100/0/100 -JX11c -Baafg+u" %" -Bbafg+u" %" -Bcagf+u" %" -B+gyellow -La/b/c -Gred -Wthin -P -K -Xc > $ps
gmt psternary p.txt -R -JX-11c -Baafg+u" %" -Bbafg+u" %" -Bcagf+u" %" -B+gyellow -La/b/c -Gred -Wthin -O -Y12c >> $ps
