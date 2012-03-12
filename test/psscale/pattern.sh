#!/bin/bash
#	$Id$
#
. functions.sh

header "Test psscale with patterns"

gmtset FONT_ANNOT_PRIMARY 10p

cat << EOF > patt.cpt
0	p144/9	5	-
5	gray	10	gray
10	p144/5	15	-
15	pink	20	pink
20	p144/30	25	-
25	black	30	black
EOF
grdmath -R-5/5/-5/5 -I1 0 0 CDIST 2.5 DIV DUP MUL NEG EXP 30 MUL = t.nc
grdview t.nc -JX5i -P -Xc -Yc -K -Cpatt.cpt -Qs -B1 > $ps
psscale -Cpatt.cpt -D2.5i/-0.5i/4.5i/0.25ih -O -K >> $ps
psscale -Cpatt.cpt -D5.5i/2.5i/4.5i/0.25i -O >> $ps

pscmp pattern
