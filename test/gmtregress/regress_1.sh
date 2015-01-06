#!/bin/bash
#       $Id$
# Testing gmtregress on the Hertzsprung-Russell data from Rousseeuw & Leroy, 19987,
# "Robust Regression and Outlier Detection", Wiley, 329pp [Table 3].
# Data is Log Temperature (x) vs Log Light Intensity (y).
# Outliers are red giants that fall off trend; there are commented out in data,
# so we use sed to include them in this script

ps=regress_1.ps

function plot_one {	# 5 args are: -E -N axes -X -Y
	# Use negative angle since we are plotting data in regress_2.sh against -x
	gmt regress -A-90/90/0.1 $1 $2 data | awk '{print -$1, $2}' > tmp
	gmt psxy -R -W0.5p,blue -J -Bxa45f15g45+u\\232 -Bya1pg1 -B$3 -O -K $4 $5 tmp
	gmt math tmp -i0,1 DUP DUP LOWER EQ MUL = | awk '{if (NF == 2 && $2 > 0) printf "%s %s %s 0.001\n", $1, $2, $1}' | gmt psxy -R -J -O -K -Sv0.2i+s+b -Gred $4 $5 --MAP_VECTOR_SHAPE=1
	
}
# Allow outliers to be included in the analysis:
sed -e s/#//g hertzsprung-russell.txt > data
gmt psxy -R-90/90/0.001/100 -JX2i/2il -T -P -K -Xa1i -Ya1i > $ps
# L1
plot_one -Er -N1 WSne -Xa1.1i -Ya1i >> $ps
plot_one -Eo -N1 Wsne -Xa1.1i -Ya3.25i >> $ps
plot_one -Ex -N1 Wsne -Xa1.1i -Ya5.5i >> $ps
plot_one -Ey -N1 WsNe+tL1 -Xa1.1i -Ya7.75i >> $ps
#L2
plot_one -Er -N2 wSne -Xa3.4i -Ya1i >> $ps
plot_one -Eo -N2 wsne -Xa3.4i -Ya3.25i >> $ps
plot_one -Ex -N2 wsne -Xa3.4i -Ya5.5i >> $ps
plot_one -Ey -N2 wsNe+tL2 -Xa3.4i -Ya7.75i >> $ps
#LMS
plot_one -Er -Nr weSn -Xa5.7i -Ya1i >>$ps
plot_one -Eo -Nr wesn -Xa5.7i -Ya3.25i >> $ps
plot_one -Ex -Nr wesn -Xa5.7i -Ya5.5i >> $ps
plot_one -Ey -Nr wesN+tLMS -Xa5.7i -Ya7.75i >> $ps
# Labels
echo 90 0.5 REDUCED MAJOR AXIS | gmt pstext -R -J -O -K -F+jTC+a90 -N -Dj0.2i -Xa5.7i -Ya1i >> $ps
echo 90 0.5 ORTHOGONAL | gmt pstext -R -J -O -K -F+jTC+a90 -N -Dj0.2i -Xa5.7i -Ya3.25i >> $ps
echo 90 0.5 X ON Y | gmt pstext -R -J -O -K -F+jTC+a90 -N -Dj0.2i -Xa5.7i -Ya5.5i >> $ps
echo 90 0.5 Y ON X | gmt pstext -R -J -O -K -F+jTC+a90 -N -Dj0.2i -Xa5.7i -Ya7.75i >> $ps
echo -90 0.001 MISFIT | gmt pstext -R -J -O -K -F+jBC+a90 -N -Dj0.5i -Xa1.1i -Ya5.5i >> $ps
gmt psxy -R -J -O -T >> $ps
