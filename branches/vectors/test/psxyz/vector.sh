#!/bin/bash
#       $Id$
#
# Check vector symbols

. ../functions.sh
header "Test psxyz with straight vector symbols"
ps=vector.ps
psbasemap -R0/6/0/3 -Jx1i -P -B1g1WSne -K -Xc -p155/35 > $ps
gmtset MAP_VECTOR_SHAPE 0.5
# Center justified vectors
echo 0.5	0.5	0	30	1i	| psxyz -R -J -O -K -W1p -Gred -Sv0.2i+jc -p155/35 >> $ps
echo 1.5	0.5	0	30	1i	| psxyz -R -J -O -K -W1p -Gred -Sv0.2i+jc+b -p155/35 >> $ps
echo 2.5	0.5	0	30	1i	| psxyz -R -J -O -K -W1p -Gred -Sv0.2i+jc+e -p155/35 >> $ps
echo 3.5	0.5	0	30	1i	| psxyz -R -J -O -K -W1p -Gred -Sv0.2i+jc+b+e -p155/35 >> $ps
echo 4.5	0.5	0	30	1i	| psxyz -R -J -O -K -W1p -Gred -Sv0.2i+jc+b+l -p155/35 >> $ps
echo 5.5	0.5	0	30	1i	| psxyz -R -J -O -K -W1p -Gred -Sv0.2i+jc+e+r -p155/35 >> $ps
# Beginning justified vectors
echo 0.1	1.2	0	30	1i	| psxyz -R -J -O -K -W1p -Gyellow -Sv0.2i+jb -p155/35 >> $ps
echo 1.1	1.2	0	30	1i	| psxyz -R -J -O -K -W1p -Gyellow -Sv0.2i+jb+b -p155/35 >> $ps
echo 2.1	1.2	0	30	1i	| psxyz -R -J -O -K -W1p -Gyellow -Sv0.2i+jb+e -p155/35 >> $ps
echo 3.1	1.2	0	30	1i	| psxyz -R -J -O -K -W1p -Gyellow -Sv0.2i+jb+b+e -p155/35 >> $ps
echo 4.1	1.2	0	30	1i	| psxyz -R -J -O -K -W1p -Gyellow -Sv0.2i+jb+b+l -p155/35 >> $ps
echo 5.1	1.2	0	30	1i	| psxyz -R -J -O -K -W1p -Gyellow -Sv0.2i+jb+e+r -p155/35 >> $ps
# End justified vectors
echo 0.9	2.8	0	30	1i	| psxyz -R -J -O -K -W1p -Sv0.2i+je -p155/35 >> $ps
echo 1.9	2.8	0	30	1i	| psxyz -R -J -O -K -W1p -Sv0.2i+je+b -p155/35 >> $ps
echo 2.9	2.8	0	30	1i	| psxyz -R -J -O -K -W1p -Sv0.2i+je+e -p155/35 >> $ps
echo 3.9	2.8	0	30	1i	| psxyz -R -J -O -K -W1p -Sv0.2i+je+b+e -p155/35 >> $ps
echo 4.9	2.8	0	30	1i	| psxyz -R -J -O -K -W1p -Sv0.2i+je+b+l -p155/35 >> $ps
echo 5.9	2.8	0	30	1i	| psxyz -R -J -O -K -W1p -Sv0.2i+je+e+r -p155/35 >> $ps
# Then with -SV and Mercator
gmtset MAP_VECTOR_SHAPE 1
psbasemap -R0/6/0/3 -Jm1i -P -B1g1WSne -O -K -Y4i -p155/35 >> $ps
# Center justified vectors
echo 0.5	0.5	0	60	1i	| psxyz -R -J -O -K -W1p -Gred -SV0.2i+jc -p155/35 >> $ps
echo 1.5	0.5	0	60	1i	| psxyz -R -J -O -K -W1p -Gred -SV0.2i+jc+b -p155/35 >> $ps
echo 2.5	0.5	0	60	1i	| psxyz -R -J -O -K -W1p -Gred -SV0.2i+jc+e -p155/35 >> $ps
echo 3.5	0.5	0	60	1i	| psxyz -R -J -O -K -W1p -Gred -SV0.2i+jc+b+e -p155/35 >> $ps
echo 4.5	0.5	0	60	1i	| psxyz -R -J -O -K -W1p -Gred -SV0.2i+jc+b+l -p155/35 >> $ps
echo 5.5	0.5	0	60	1i	| psxyz -R -J -O -K -W1p -Gred -SV0.2i+jc+e+r -p155/35 >> $ps
# Beginning justified vectors
echo 0.1	1.2	0	60	1i	| psxyz -R -J -O -K -W1p -Gyellow -SV0.2i+jb -p155/35 >> $ps
echo 1.1	1.2	0	60	1i	| psxyz -R -J -O -K -W1p -Gyellow -SV0.2i+jb+b -p155/35 >> $ps
echo 2.1	1.2	0	60	1i	| psxyz -R -J -O -K -W1p -Gyellow -SV0.2i+jb+e -p155/35 >> $ps
echo 3.1	1.2	0	60	1i	| psxyz -R -J -O -K -W1p -Gyellow -SV0.2i+jb+b+e -p155/35 >> $ps
echo 4.1	1.2	0	60	1i	| psxyz -R -J -O -K -W1p -Gyellow -SV0.2i+jb+b+l -p155/35 >> $ps
echo 5.1	1.2	0	60	1i	| psxyz -R -J -O -K -W1p -Gyellow -SV0.2i+jb+e+r -p155/35 >> $ps
# End justified vectors
echo 0.9	2.8	0	60	1i	| psxyz -R -J -O -K -W1p -SV0.2i+je -p155/35 >> $ps
echo 1.9	2.8	0	60	1i	| psxyz -R -J -O -K -W1p -SV0.2i+je+b -p155/35 >> $ps
echo 2.9	2.8	0	60	1i	| psxyz -R -J -O -K -W1p -SV0.2i+je+e -p155/35 >> $ps
echo 3.9	2.8	0	60	1i	| psxyz -R -J -O -K -W1p -SV0.2i+je+b+e -p155/35 >> $ps
echo 4.9	2.8	0	60	1i	| psxyz -R -J -O -K -W1p -SV0.2i+je+b+l -p155/35 >> $ps
echo 5.9	2.8	0	60	1i	| psxyz -R -J -O -K -W1p -SV0.2i+je+e+r -p155/35 >> $ps

psxy -R -J -O -T >> $ps

pscmp
