#!/usr/bin/env bash
cat << EOF > lines.txt
0       0
5       0
EOF

gmt begin GMT_linecap
	gmt plot lines.txt -R-0.25/5.25/-0.2/1.4 -Jx1i -W4p
	gmt plot lines.txt -Y0.2i -W4p,orange,.
	gmt plot lines.txt -Y0.2i -W4p,red,9_4_2_4:2p
	gmt plot lines.txt -Y0.2i -W4p,-  --PS_LINE_CAP=round
	gmt plot lines.txt -Y0.2i -W4p,orange,0_8 --PS_LINE_CAP=round
	gmt plot lines.txt -Y0.2i -W4p,red,0_16  --PS_LINE_CAP=round
	gmt plot lines.txt -W2p,green,0_16:8p  --PS_LINE_CAP=round
gmt end show
