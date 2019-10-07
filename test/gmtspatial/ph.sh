#!/usr/bin/env bash
#	Test gmtspatial hold/perimeter function
# File with polygons, some are holes, none marked or reversed
ps=ph.ps
cat << EOF > raw.txt
> Big one
0	0
5	0
5	5
0	5
> Big two
10	0
15	0
15	5
10	5
> Little one1
1	1
2	1
2	2
1	2
> Little two1
3	1
4	1
4	2
3	2
> Little one2
11	1
12	1
12	2
11	2
> Little two2
13	1
14	1
14	2
13	2
EOF
gmt spatial -Sh raw.txt > organized.txt

gmt psxy -R-0.5/15.5/-1/6 -Jx1c -P -Baf -B+t"Original" -Gred -W1p raw.txt -K > $ps
gmt psxy -R -J -Baf -Gred -W1p -B+t"Organized" organized.txt -O -Y10c >> $ps
