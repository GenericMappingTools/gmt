#!/bin/bash
#	$Id$
#

. functions.sh
header "Test pscontour for ticked contours"

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

pscontour t.txt -R-2/11/-1/6 -Jx0.5i -B2g1 -C1 -A2 -Wa1p,red -Wc0.25p,blue -L0.25p,- -GlZ-/Z+ -P -K -T0.1i/0.05i:LH > $ps
psxy t.txt -R -J -Sc0.05i -Gblack -O -K >> $ps
makecpt -T0/10/1 -Crainbow > t.cpt
pscontour t.txt -R-2/11/-1/6 -Jx0.5i -B2g1 -Ct.cpt -I -W0.25p -GlZ-/Z+ -O -K -Y4.25i >> $ps
psxy t.txt -R -J -Sc0.1i -W2p -O >> $ps

pscmp
