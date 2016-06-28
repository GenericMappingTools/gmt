#!/bin/bash
#               GMT EXAMPLE 47
#               $Id$
#
# Purpose:      Illustrate use of gmtregress with different norms and types
# GMT modules:  gmtregress, psxy
#

ps=example_47.ps

function plot_one {	# 5 args are: -E -N axes -X -Y
	gmt regress data -Fxm $1 $2 -T2.85/5.25/0.1 > tmp
	gmt psxy -R -J -Bxafg+l"Log temperature" -Byafg+l"Log light intensity" -B$3+ghoneydew -O -K $4 $5 data -Sc0.05i -Gblue
	gmt psxy -R -J -O -K $4 $5 giants -Sc0.05i -Gred -N
	gmt psxy -R -J -O -K $4 $5 giants -Sc0.1i -W0.25p -N
	gmt psxy -R -W2p -J -O -K $4 $5 tmp	
}
# Allow outliers (commented out by #) to be included in the analysis:
sed -e s/#//g hertzsprung-russell.txt > data
# Identify the red giants (outliers)
grep '#' hertzsprung-russell.txt | sed -e s/#//g > giants

gmt psxy -R2.85/5.25/3.9/6.3 -JX-2i/2i -T -P -K -Xa1i -Ya1i > $ps
# L1
plot_one -Er -N1 WSne -Xa1.2i -Ya01i >> $ps
plot_one -Eo -N1 Wsne -Xa1.2i -Ya3.2i >> $ps
plot_one -Ex -N1 Wsne -Xa1.2i -Ya5.4i >> $ps
plot_one -Ey -N1 Wsne+tL@-1@- -Xa1.2i -Ya7.6i >> $ps
#L2
plot_one -Er -N2 wSne -Xa3.3i -Ya1i >> $ps
plot_one -Eo -N2 wsne -Xa3.3i -Ya3.2i >> $ps
plot_one -Ex -N2 wsne -Xa3.3i -Ya5.4i >> $ps
plot_one -Ey -N2 wsne+tL@-2@- -Xa3.3i -Ya7.6i >> $ps
#LMS
plot_one -Er -Nr weSn -Xa5.4i -Ya1i >>$ps
plot_one -Eo -Nr wesn -Xa5.4i -Ya3.2i >> $ps
plot_one -Ex -Nr wesn -Xa5.4i -Ya5.4i >> $ps
plot_one -Ey -Nr wesn+tLMS -Xa5.4i -Ya7.6i >> $ps
# Labels
echo 2.85 5.1 REDUCED MAJOR AXIS | gmt pstext -R -J -O -K -F+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya1i >> $ps
echo 2.85 5.1 ORTHOGONAL | gmt pstext -R -J -O -K -F+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya3.2i >> $ps
echo 2.85 5.1 X ON Y | gmt pstext -R -J -O -K -F+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya5.4i >> $ps
echo 2.85 5.1 Y ON X | gmt pstext -R -J -O -K -F+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya7.6i >> $ps
gmt psxy -R -J -O -T >> $ps
rm -f tmp data giants
