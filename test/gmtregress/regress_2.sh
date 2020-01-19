#!/usr/bin/env bash
# Testing gmtregress on the Hertzsprung-Russell data from Rousseeuw & Leroy, 19987,
# "Robust Regression and Outlier Detection", Wiley, 329pp [Table 3].
# Data is Log Temperature (x) vs Log Light Intensity (y).
# Outliers are red giants that fall off trend; there are commented out in data,
# so we use sed to identify them.
# This script shows the best-fits for each combination of -E and -N.

ps=regress_2.ps

function plot_one {	# 5 args are: -E -N axes -X -Y
	gmt regress data -Fxm $1 $2 -T2.85/5.25/0.1 > tmp
	gmt psxy -R -J -Bxafg -Byafg -B$3 -O -K $4 $5 data -Sc0.05i -Gblue
	gmt psxy -R -J -O -K $4 $5 giants -Sc0.1i -W0.25p -N
	gmt psxy -R -W1p,red -J -O -K $4 $5 tmp -W1p
}
# Allow outliers to be included in the analysis:
file=`gmt which -G @hertzsprung-russell.txt`
sed -e s/#//g $file > data
# Identify the red giants (outliers)
grep '#' $file | sed -e s/#//g > giants

gmt psxy -R2.85/5.25/3.9/6.3 -JX-2i/2i -T -P -K -Xa1i -Ya1i > $ps
# L1
plot_one -Er -N1 WSne -Xa1.2i -Ya01i >> $ps
plot_one -Eo -N1 Wsne -Xa1.2i -Ya3.25i >> $ps
plot_one -Ex -N1 Wsne -Xa1.2i -Ya5.5i >> $ps
plot_one -Ey -N1 WsNe+tL1 -Xa1.2i -Ya7.75i >> $ps
#L2
plot_one -Er -N2 wSne -Xa3.3i -Ya1i >> $ps
plot_one -Eo -N2 wsne -Xa3.3i -Ya3.25i >> $ps
plot_one -Ex -N2 wsne -Xa3.3i -Ya5.5i >> $ps
plot_one -Ey -N2 wsNe+tL2 -Xa3.3i -Ya7.75i >> $ps
#LMS
plot_one -Er -Nr weSn -Xa5.4i -Ya1i >>$ps
plot_one -Eo -Nr wesn -Xa5.4i -Ya3.25i >> $ps
plot_one -Ex -Nr wesn -Xa5.4i -Ya5.5i >> $ps
plot_one -Ey -Nr wesN+tLMS -Xa5.4i -Ya7.75i >> $ps
# Labels
echo 2.85 5.1 REDUCED MAJOR AXIS | gmt pstext -R -J -O -K -F+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya1i >> $ps
echo 2.85 5.1 ORTHOGONAL | gmt pstext -R -J -O -K -F+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya3.25i >> $ps
echo 2.85 5.1 X ON Y | gmt pstext -R -J -O -K -F+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya5.5i >> $ps
echo 2.85 5.1 Y ON X | gmt pstext -R -J -O -K -F+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya7.75i >> $ps
gmt psxy -R -J -O -T >> $ps
