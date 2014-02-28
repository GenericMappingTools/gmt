#!/bin/bash
#       $Id$
#
# Check geovector symbols for small circle vectors

ps=smallcirvec.ps
gmt gmtset PROJ_ELLIPSOID Sphere
# Vector from point with head at end
cat << EOF > t.txt
0 0 8000	red	0	45	b+e+b
-30 0 6000	green	-30	-30	e+e+b
30 30 6000	blue	10	10	c+e+b
-10 0 6671.7	yellow	80	0	c+e+b
EOF
gmt psbasemap -Rg -JG0/0/4.5i -Bag -P -K -Xc > $ps
while read lon lat length color plon plat just; do
	echo $lon $lat $length | gmt psxy -R -J -W1p,$color -S=0.4i+j${just}+o${plon}/${plat} -G$color -O -K --MAP_VECTOR_SHAPE=1 >> $ps
	echo $lon $lat   | gmt psxy -R -J -O -K -Sc0.1i -G$color -W0.25p >> $ps
	echo $plon $plat | gmt psxy -R -J -O -K -St0.1i -G$color -W0.25p >> $ps
done < t.txt
# Vector from point with head at end
cat << EOF > t.txt
-20	-20	-45	30	red	5	-5	b+b
30	-30	0	60	blue	-15	15	b+b+e+r
0	45	-90	180	green	0	0	b+e
EOF
gmt psbasemap -R -J -Bag -O -K -Y4.75i >> $ps
while read lon lat angle1 angle2 color plon plat just; do
	echo $lon $lat $angle1 $angle2 | gmt psxy -R -J -W2p,$color -S=0.4i+j${just}+o${plon}/${plat}+q+p0.5p -G$color -O -K --MAP_VECTOR_SHAPE=1 >> $ps
	echo $lon $lat   | gmt psxy -R -J -O -K -Sc0.1i -G$color -W0.25p >> $ps
	echo $plon $plat | gmt psxy -R -J -O -K -St0.1i -G$color -W0.25p >> $ps
done < t.txt
gmt psxy -R -J -O -T >> $ps
