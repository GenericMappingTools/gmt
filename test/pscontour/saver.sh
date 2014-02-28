#!/bin/bash
#	$Id$
#

ps=saver.ps

cat << EOF > t.txt
0	2	5
1	4	5
2	0.5	5
3	3	9
4	4.5	5
4.2	1.2	5
6	3	1
8	1	5
9	4.5	5
EOF

gmt pscontour t.txt -C1 -A2 -Ddump
gmt psxy t.txt -R-2/11/-1/6 -Jx0.5i -Ss0.1i -Gred -P -K -Xc > $ps
gmt psxy dump -R -J -Sc0.05i -Gblue -O -K >> $ps
gmt pscontour t.txt -R -J -B2g1 -C1 -A1 -W0.25p -Gl6.1/-1/6.1/6 -O >> $ps
