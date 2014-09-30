#!/bin/bash
# Test proper drawing of math angles
ps=matharrow.ps
r=1.75c
gmt psxy -R-5/5/-8/8 -Jx0.6i -Sm0.3i+b+e+r -W0.5p,red -Gblue -Bag3 -P -Xc << EOF > $ps
-3 -6 $r 5 85
-3 -3 $r 20 135
-3 0 $r 30 174
-3 3 $r 40 226
-3 6 $r 50 349
0 -6 $r 100 164
0 -3 $r 120 177
0 0 $r 130 238
0 3 $r 140 290
0 6 $r 150 355
3 -6 $r 240 305
3 -3 $r 260 320
3 0 $r 270 355
3 3 $r 280 390
3 6 $r 290 420
EOF
