#!/bin/sh
# Test psxy lines with -g
. ../functions.sh
ps=new_gaps.ps
header "Test plotting line graphs with different gap options"
cat << EOF >> $$.d
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
psxy -R0/12/0/10 -JX3i -B5g1WSne -P -Y6i $$.d -W2p -K > $ps
psxy -R -J -O -K -Sc0.1i -Gred $$.d >> $ps
# Test -g in x
psxy -R -J -O -K -X3.5i -B5g1WSne $$.d -W2p -gx1.5 >> $ps
psxy -R -J -O -K -Sc0.1i -Gred $$.d >> $ps
echo "0 10 18 0 0 LT -gx1.5" | pstext -R -J -O -K -Wwhite,o0.25p -Dj0.1i/0.1i >> $ps
# Test -g in y
psxy -R -J -O -K -X-3.5i -B5g1WSne -Y-3.5i $$.d -W2p -gy1.5 >> $ps
psxy -R -J -O -K -Sc0.1i -Gred $$.d >> $ps
echo "0 10 18 0 0 LT -gy1.5" | pstext -R -J -O -K -Wwhite,o0.25p -Dj0.1i/0.1i >> $ps
# Test -g in d
psxy -R -J -O -K -X3.5i -B5g1WSne $$.d -W2p -gd1.5 >> $ps
psxy -R -J -O -K -Sc0.1i -Gred $$.d >> $ps
echo "0 10 18 0 0 LT -gd1.5" | pstext -R -J -O -Wwhite,o0.25p -Dj0.1i/0.1i >> $ps

pscmp
rm -f $$.*
