#!/bin/bash
#       $Id$
#
# Check vector symbols

. ../functions.sh
header "Test psxy with math angle vector symbols"
ps=matharc.ps
psbasemap -R0/6/0/3 -Jx1i -P -B1g1WSne -K -Xc > $ps
gmtset MAP_VECTOR_SHAPE 1
# Math angle vectors
echo 0	0	1i	30	80	| psxy -R -J -O -K -W1p -Gred -Sm0.2i >> $ps
echo 1	0	1i	30	80	| psxy -R -J -O -K -W1p -Gred -Sm0.2i+b >> $ps
echo 2	0	1i	30	80	| psxy -R -J -O -K -W1p -Gred -Sm0.2i+e >> $ps
echo 3	0	1i	30	80	| psxy -R -J -O -K -W1p -Gred -Sm0.2i+b+e >> $ps
echo 4	0	1i	30	80	| psxy -R -J -O -K -W1p -Gred -Sm0.2i+b+l >> $ps
echo 5	0	1i	30	80	| psxy -R -J -O -K -W1p -Gred -Sm0.2i+e+r >> $ps
# Right angles
echo 0.5	1.5	0.5i	0	90	| psxy -R -J -O -K -W1p -Gred -SM0.2i >> $ps
echo 1.5	1.5	0.5i	60	150	| psxy -R -J -O -K -W1p -Gred -SM0.2i+b >> $ps
echo 2.5	1.5	0.5i	120	210	| psxy -R -J -O -K -W1p -Gred -SM0.2i+e >> $ps
echo 3.5	1.5	0.5i	180	270	| psxy -R -J -O -K -W1p -Gred -SM0.2i+b+e >> $ps
echo 4.5	1.5	0.5i	240	330	| psxy -R -J -O -K -W1p -Gred -SM0.2i+b+l >> $ps
echo 5.5	1.5	0.5i	300	390	| psxy -R -J -O -K -W1p -Gred -SM0.2i+e+r >> $ps
# Math angle vectors unfilled
echo 0	2	1i	30	80	| psxy -R -J -O -K -W1p -Sm0.2i >> $ps
echo 1	2	1i	30	80	| psxy -R -J -O -K -W1p -Sm0.2i+b >> $ps
echo 2	2	1i	30	80	| psxy -R -J -O -K -W1p -Sm0.2i+e >> $ps
echo 3	2	1i	30	80	| psxy -R -J -O -K -W1p -Sm0.2i+b+e >> $ps
echo 4	2	1i	30	80	| psxy -R -J -O -K -W1p -Sm0.2i+b+l >> $ps
echo 5	2	1i	30	80	| psxy -R -J -O -K -W1p -Sm0.2i+e+r >> $ps
# Normalized by angle below
psbasemap -R0/4/0/4 -J -O -B1g1WSne -K -X1i -Y4i >> $ps
psxy -R -J -O -K -W1p -Gblack -Sm0.3i+b+e+n90 << EOF >> $ps
0	0	4.0i	0	90
0	0	3.6i	0	80
0	0	3.2i	0	70
0	0	2.8i	0	60
0	0	2.4i	0	50
0	0	2.0i	0	40
0	0	1.8i	0	35
0	0	1.6i	0	30
0	0	1.4i	0	25
0	0	1.2i	0	20
0	0	1.0i	0	15
0	0	0.8i	0	10
EOF
psxy -R -J -O -T >> $ps

pscmp
