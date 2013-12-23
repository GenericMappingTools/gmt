#!/bin/bash
# Test gmt gmtconvert with -g
# Same plotting as gmt psxy/new_gaps.sh but using gmt gmtconvert to make the gaps.

ps=new_gaps2.ps

cat << EOF > tt.d
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
gmt gmtconvert tt.d -gx1.5 | gmt psxy -R -J -O -K -X3.5i -B5g1 -BWSne -W2p >> $ps
gmt psxy -R -J -O -K -Sc0.1i -Gred tt.d >> $ps
gmt pstext -R -J -O -K -F+f18p+jLT -Gwhite -W -Dj0.1i/0.1i >> $ps <<< "0 10 -gx1.5"
# Test -g in y
gmt gmtconvert tt.d -gy1.5 | gmt psxy -R -J -O -K -X-3.5i -B5g1 -BWSne -Y-3.5i -W2p >> $ps
gmt psxy -R -J -O -K -Sc0.1i -Gred tt.d >> $ps
gmt pstext -R -J -O -K -F+f18p+jLT -Gwhite -W -Dj0.1i/0.1i >> $ps <<< "0 10 -gy1.5"
# Test -g in d
gmt gmtconvert tt.d -gd1.5 | gmt psxy -R -J -O -K -X3.5i -B5g1 -BWSne -W2p >> $ps
gmt psxy -R -J -O -K -Sc0.1i -Gred tt.d >> $ps
gmt pstext -R -J -O -F+f18p+jLT -Gwhite -W -Dj0.1i/0.1i >> $ps <<< "0 10 -gd1.5"

