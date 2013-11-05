#!/bin/bash
# Test gmtconvert with -g
# Same plotting as psxy/new_gaps.sh but using gmtconvert to make the gaps.

. functions.sh
header "Let gmtconvert make gaps in series"

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
gmtconvert tt.d -gx1.5 | psxy -R -J -O -K -X3.5i -B5g1WSne -W2p >> $ps
psxy -R -J -O -K -Sc0.1i -Gred tt.d >> $ps
pstext -R -J -O -K -F+f18p+jLT -Gwhite -W -Dj0.1i/0.1i >> $ps <<< "0 10 -gx1.5"
# Test -g in y
gmtconvert tt.d -gy1.5 | psxy -R -J -O -K -X-3.5i -B5g1WSne -Y-3.5i -W2p >> $ps
psxy -R -J -O -K -Sc0.1i -Gred tt.d >> $ps
pstext -R -J -O -K -F+f18p+jLT -Gwhite -W -Dj0.1i/0.1i >> $ps <<< "0 10 -gy1.5"
# Test -g in d
gmtconvert tt.d -gd1.5 | psxy -R -J -O -K -X3.5i -B5g1WSne -W2p >> $ps
psxy -R -J -O -K -Sc0.1i -Gred tt.d >> $ps
pstext -R -J -O -F+f18p+jLT -Gwhite -W -Dj0.1i/0.1i >> $ps <<< "0 10 -gd1.5"

pscmp
