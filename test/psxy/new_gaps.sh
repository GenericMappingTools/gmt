#!/bin/bash
# Test psxy lines with -g
. functions.sh
ps=new_gaps.ps
header "Test plotting line graphs with different gap options"
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
psxy -R0/12/0/10 -JX3i -B5g1WSne -P -Y6i tt.d -W2p -K > $ps
psxy -R -J -O -K -Sc0.1i -Gred tt.d >> $ps
# Test -g in x
psxy -R -J -O -K -X3.5i -B5g1WSne tt.d -W2p -gx1.5 >> $ps
psxy -R -J -O -K -Sc0.1i -Gred tt.d >> $ps
pstext -R -J -F+f18p+jTL -O -K -Gwhite -W -Dj0.1i >> $ps <<< "0 10 -gx1.5"
# Test -g in y
psxy -R -J -O -K -X-3.5i -B5g1WSne -Y-3.5i tt.d -W2p -gy1.5 >> $ps
psxy -R -J -O -K -Sc0.1i -Gred tt.d >> $ps
pstext -R -J -F+f18p+jTL -O -K -Gwhite -W -Dj0.1i >> $ps <<< "0 10 -gy1.5"
# Test -g in d
psxy -R -J -O -K -X3.5i -B5g1WSne tt.d -W2p -gd1.5 >> $ps
psxy -R -J -O -K -Sc0.1i -Gred tt.d >> $ps
pstext -R -J -F+f18p+jTL -O -Gwhite -W -Dj0.1i >> $ps <<< "0 10 -gd1.5"

pscmp
