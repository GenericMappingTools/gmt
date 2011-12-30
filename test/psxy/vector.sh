#!/bin/bash
#       $Id$
#
# Check vector symbols

. ../functions.sh
header "Test psxy with straight vector symbols"
ps=vector.ps
psbasemap -R0/6/0/3 -Jx1i -P -B1g1WSne -K -Xc > $ps
gmtset MAP_VECTOR_SHAPE 0.5
# Center justified vectors
echo 0.5	0.5	30	1i	| psxy -R -J -O -K -W1p -Gred -Sv0.2i+jc >> $ps
echo 1.5	0.5	30	1i	| psxy -R -J -O -K -W1p -Gred -Sv0.2i+jc+b >> $ps
echo 2.5	0.5	30	1i	| psxy -R -J -O -K -W1p -Gred -Sv0.2i+jc+e >> $ps
echo 3.5	0.5	30	1i	| psxy -R -J -O -K -W1p -Gred -Sv0.2i+jc+b+e >> $ps
echo 4.5	0.5	30	1i	| psxy -R -J -O -K -W1p -Gred -Sv0.2i+jc+b+l >> $ps
echo 5.5	0.5	30	1i	| psxy -R -J -O -K -W1p -Gred -Sv0.2i+jc+e+r >> $ps
# Beginning justified vectors
echo 0.1	1.2	30	1i	| psxy -R -J -O -K -W1p -Gyellow -Sv0.2i+jb >> $ps
echo 1.1	1.2	30	1i	| psxy -R -J -O -K -W1p -Gyellow -Sv0.2i+jb+b >> $ps
echo 2.1	1.2	30	1i	| psxy -R -J -O -K -W1p -Gyellow -Sv0.2i+jb+e >> $ps
echo 3.1	1.2	30	1i	| psxy -R -J -O -K -W1p -Gyellow -Sv0.2i+jb+b+e >> $ps
echo 4.1	1.2	30	1i	| psxy -R -J -O -K -W1p -Gyellow -Sv0.2i+jb+b+l >> $ps
echo 5.1	1.2	30	1i	| psxy -R -J -O -K -W1p -Gyellow -Sv0.2i+jb+e+r >> $ps
# End justified vectors
echo 0.9	2.8	30	1i	| psxy -R -J -O -K -W1p -Sv0.2i+je >> $ps
echo 1.9	2.8	30	1i	| psxy -R -J -O -K -W1p -Sv0.2i+je+b >> $ps
echo 2.9	2.8	30	1i	| psxy -R -J -O -K -W1p -Sv0.2i+je+e >> $ps
echo 3.9	2.8	30	1i	| psxy -R -J -O -K -W1p -Sv0.2i+je+b+e >> $ps
echo 4.9	2.8	30	1i	| psxy -R -J -O -K -W1p -Sv0.2i+je+b+l >> $ps
echo 5.9	2.8	30	1i	| psxy -R -J -O -K -W1p -Sv0.2i+je+e+r >> $ps
# Then with -SV and Mercator
gmtset MAP_VECTOR_SHAPE 1
psbasemap -R0/6/0/3 -Jm1i -P -B1g1WSne -O -K -Y4i >> $ps
# Center justified vectors
echo 0.5	0.5	60	1i	| psxy -R -J -O -K -W1p -Gred -SV0.2i+jc >> $ps
echo 1.5	0.5	60	1i	| psxy -R -J -O -K -W1p -Gred -SV0.2i+jc+b >> $ps
echo 2.5	0.5	60	1i	| psxy -R -J -O -K -W1p -Gred -SV0.2i+jc+e >> $ps
echo 3.5	0.5	60	1i	| psxy -R -J -O -K -W1p -Gred -SV0.2i+jc+b+e >> $ps
echo 4.5	0.5	60	1i	| psxy -R -J -O -K -W1p -Gred -SV0.2i+jc+b+l >> $ps
echo 5.5	0.5	60	1i	| psxy -R -J -O -K -W1p -Gred -SV0.2i+jc+e+r >> $ps
# Beginning justified vectors
echo 0.1	1.2	60	1i	| psxy -R -J -O -K -W1p -Gyellow -SV0.2i+jb >> $ps
echo 1.1	1.2	60	1i	| psxy -R -J -O -K -W1p -Gyellow -SV0.2i+jb+b >> $ps
echo 2.1	1.2	60	1i	| psxy -R -J -O -K -W1p -Gyellow -SV0.2i+jb+e >> $ps
echo 3.1	1.2	60	1i	| psxy -R -J -O -K -W1p -Gyellow -SV0.2i+jb+b+e >> $ps
echo 4.1	1.2	60	1i	| psxy -R -J -O -K -W1p -Gyellow -SV0.2i+jb+b+l >> $ps
echo 5.1	1.2	60	1i	| psxy -R -J -O -K -W1p -Gyellow -SV0.2i+jb+e+r >> $ps
# End justified vectors
echo 0.9	2.8	60	1i	| psxy -R -J -O -K -W1p -SV0.2i+je >> $ps
echo 1.9	2.8	60	1i	| psxy -R -J -O -K -W1p -SV0.2i+je+b >> $ps
echo 2.9	2.8	60	1i	| psxy -R -J -O -K -W1p -SV0.2i+je+e >> $ps
echo 3.9	2.8	60	1i	| psxy -R -J -O -K -W1p -SV0.2i+je+b+e >> $ps
echo 4.9	2.8	60	1i	| psxy -R -J -O -K -W1p -SV0.2i+je+b+l >> $ps
echo 5.9	2.8	60	1i	| psxy -R -J -O -K -W1p -SV0.2i+je+e+r >> $ps

psxy -R -J -O -T >> $ps

pscmp
