#!/bin/bash
#       $Id$
#
# Check vector symbols

. ../functions.sh
header "Test psxy with math angle vector symbols"
ps=matharc.ps
psbasemap -R0/6/0/3 -Jx1i -P -B1g1WSne -K -Xc > $ps
gmtset MAP_VECTOR_SHAPE 0.5
# Center justified vectors
echo 0	0	1i	30	80	| psxy -R -J -O -K -W1p -Gred -Sm0.2i >> $ps
echo 1	0	1i	30	80	| psxy -R -J -O -K -W1p -Gred -Sm0.2i+b >> $ps
echo 2	0	1i	30	80	| psxy -R -J -O -K -W1p -Gred -Sm0.2i+e >> $ps
echo 3	0	1i	30	80	| psxy -R -J -O -K -W1p -Gred -Sm0.2i+b+e >> $ps
echo 4	0	1i	30	80	| psxy -R -J -O -K -W1p -Gred -Sm0.2i+b+l >> $ps
echo 5	0	1i	30	80	| psxy -R -J -O -K -W1p -Gred -Sm0.2i+e+r >> $ps
psxy -R -J -O -T >> $ps

pscmp
