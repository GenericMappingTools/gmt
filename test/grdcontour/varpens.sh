#!/usr/bin/env bash
#
# Test variable pens given to -C

ps=varpens.ps
cat << EOF > pens.txt
0	A	0	1p,red
1	C	0	0.5p,blue,-
2	C	0	1p,-
3	C	0	1.5p,black
4	C	0	faint
EOF
gmt grdcontour -R156:15W/154:45W/18:45N/20:30N -JM6i @earth_relief_01m -Z+s0.001 -Cpens.txt -P -Baf -Xc > $ps
