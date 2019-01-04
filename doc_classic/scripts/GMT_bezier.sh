#!/usr/bin/env bash
#
ps=GMT_bezier.ps
cat << EOF > line.txt
0       0
1	1
2	0.5
4	2
2	1.5
EOF

gmt psxy line.txt -R-0.25/4.25/-0.2/2.2 -JX3i/1.25i -P -W2p -K > $ps
gmt psxy line.txt -R -J -O -K -Sc0.1i -Gred -Wfaint >> $ps
gmt psxy line.txt -R -J -O -W2p+s -K -X3i >> $ps
gmt psxy line.txt -R -J -O -Sc0.1i -Gred -Wfaint >> $ps
