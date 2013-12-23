#!/bin/bash
# Test gmt psxy lines with -g

ps=new_gaps.ps

cat << EOF >> tt.d
1	1
2	1
3	2
4	5
5	4
6	3
8	4
9	3
10	5
11	8
EOF
gmt psxy -R0/12/0/10 -JX3i -B5g1 -BWSne -P -Y6i tt.d -W2p -K > $ps
gmt psxy -R -J -O -K -Sc0.1i -Gred tt.d >> $ps
# Test -g in x
gmt psxy -R -J -O -K -X3.5i -B5g1 -BWSne tt.d -W2p -gx1.5 >> $ps
gmt psxy -R -J -O -K -Sc0.1i -Gred tt.d >> $ps
gmt pstext -R -J -F+f18p+jTL -O -K -Gwhite -W -Dj0.1i >> $ps <<< "0 10 -gx1.5"
# Test -g in y
gmt psxy -R -J -O -K -X-3.5i -B5g1 -BWSne -Y-3.5i tt.d -W2p -gy1.5 >> $ps
gmt psxy -R -J -O -K -Sc0.1i -Gred tt.d >> $ps
gmt pstext -R -J -F+f18p+jTL -O -K -Gwhite -W -Dj0.1i >> $ps <<< "0 10 -gy1.5"
# Test -g in d
gmt psxy -R -J -O -K -X3.5i -B5g1 -BWSne tt.d -W2p -gd1.5 >> $ps
gmt psxy -R -J -O -K -Sc0.1i -Gred tt.d >> $ps
gmt pstext -R -J -F+f18p+jTL -O -Gwhite -W -Dj0.1i >> $ps <<< "0 10 -gd1.5"

