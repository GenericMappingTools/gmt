#!/usr/bin/env bash
# Test a 2-color binary pie chart custom symbol
ps=binarypie.ps
cat << EOF > pie.def
N: 1o
0 0 1 0 \$1 w -Gblack -W-
0 0 1 \$1 360 w -Gred -W-
EOF
gmt math -T0/360/30 -N3 T -C0 COSD -C1 SIND = t.txt
gmt psxy -R-1.5/1.5/-1.5/1.5 -JX6i -P -Baf -Xc -Skpie/0.75i t.txt > $ps
