#!/bin/bash
#       $Id$
#
# Check geovector symbols for small circle vectors

ps=smallcirvec.ps
gmtset PROJ_ELLIPSOID Sphere
# Vector from point with head at end
cat << EOF > t.txt
0 0 5000	red	0	45	b
-30 0 5003.77858805	green	-30	-30	e
30 30 5000	blue	10	10	c
-10 0 6671.7	yellow	80	0	c
EOF
psbasemap -Rg -JG0/0/4.5i -Bag -P -K -Xc > $ps
while read lon lat length color plon plat just; do
	echo $lon $lat $length | psxy -R -J -W1p,$color -S=0.4i+j${just}+o${plon}/${plat} -G$color -O -K --MAP_VECTOR_SHAPE=1 >> $ps
	echo $lon $lat   | psxy -R -J -O -K -Sc0.1i -G$color -W0.25p >> $ps
	echo $plon $plat | psxy -R -J -O -K -St0.1i -G$color -W0.25p >> $ps
done < t.txt
# Vector from point with head at end
cat << EOF > t.txt
-20	-20	-15	30	red	5	-5	b
30	-30	0	30	blue	-15	15	b
EOF
psbasemap -R -J -Bag -O -K -Y4.75i >> $ps
while read lon lat angle1 angle2 color plon plat just; do
	echo $lon $lat $angle1 $angle2 | psxy -R -J -W1p,$color -S=0.4i+j${just}+o${plon}/${plat}+q -G$color -O -K --MAP_VECTOR_SHAPE=1 >> $ps
	echo $lon $lat   | psxy -R -J -O -K -Sc0.1i -G$color -W0.25p >> $ps
	echo $plon $plat | psxy -R -J -O -K -St0.1i -G$color -W0.25p >> $ps
done < t.txt
psxy -R -J -O -T >> $ps
