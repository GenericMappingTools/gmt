#!/usr/bin/env bash
# Test stereonet -D across the rim and with a dashed pen on Schmidt counts that land exactly on the levels

ps=stereonet_density.ps

cat << EOF > rim_test.txt
88 3
90 2
92 4
268 3
270 2
272 4
EOF
awk 'BEGIN {for (i = 0; i < 100; i++) printf "%d %d\n", 30 + (i * 7) % 31, 50 + (i * 11) % 21}' > planes.txt

gmt psstereonet rim_test.txt -JA7c -B -Tl -De+i1+s1 -Sc0.1c -Gblack -P -K > $ps
gmt psstereonet planes.txt -JA7c -B -Ds+i10+p1p,red,dashed -Sc0.05c -Gblack -O -X8c >> $ps
