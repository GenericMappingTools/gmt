#!/bin/bash
#	$Id$
#
# Plot error bars and test [+|-]<epen>

. ./functions.sh
header "Test psxy error bar colors"

cat << EOF > tt.d
1	1	1	1
2	2	2	1
3	3	3	1
4	4	4	1
5	5	5	1
EOF
makecpt -Crainbow -T0/6/1 > tt.cpt
psxy -R0/6/0/6 -JX3i -P -B0 -Sc0.2i -Ctt.cpt -W0.25p -X1i -Y2i tt.d -Ex/2p,red -K > $ps
psxy -R -J -O -B0 -Sc0.2i -Ctt.cpt -W0.25p -X3.25i tt.d -Ey/-1p -K >> $ps
psxy -R -J -O -B0 -Sc0.2i -Ctt.cpt -W+5p -X-3.25i -Y3.5i tt.d -Ey/+1p -K >> $ps
psxy -R -J -O -B0 -Sc0.2i -Ctt.cpt -W0.25p,red -X3.25i tt.d -Ex/-1p >> $ps

pscmp
