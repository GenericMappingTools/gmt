#!/usr/bin/env bash
cat << EOF > line.txt
0       0
1	1
2	0.5
4	2
2	1.5
EOF

gmt begin GMT_bezier
	gmt plot line.txt -R-0.25/4.25/-0.2/2.2 -JX3i/1.25i -W2p
	gmt plot line.txt -Sc0.1i -Gred -Wfaint
	gmt plot line.txt -W2p+s -X3i
	gmt plot line.txt -Sc0.1i -Gred -Wfaint
gmt end show
