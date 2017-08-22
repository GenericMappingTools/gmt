#!/bin/bash
#               GMT EXAMPLE 47
#               $Id$
#
# Purpose:      Illustrate use of gmtregress with different norms and types
# GMT modules:  gmtregress, psxy
#

export GMT_PPID=1
function plot_one {	# 2 args are: -E -N
	gmt regress data -Fxm $1 $2 -T2.85/5.25/0.1 > tmp
	gmt psxy -R2.85/5.25/3.9/6.3 -JX-2i/2i data -Sc0.05i -Gblue
	gmt psxy giants -Sc0.05i -Gred -N
	gmt psxy giants -Sc0.1i -W0.25p -N
	gmt psxy -W2p tmp	
}

gmt begin ex47 ps
  gmt which -Gl @hertzsprung-russell.txt
  # Allow outliers (commented out by #) to be included in the analysis:
  sed -e s/#//g hertzsprung-russell.txt > data
  # Identify the red giants (outliers)
  grep '#' hertzsprung-russell.txt | sed -e s/#//g > giants
  gmt subplot begin 4x2 -M0.05i -Fp2i/2i -LRl+l"Log light intensity" -LCb+l"Log temperature"+tc
  # L1 y on x
  gmt psbasemap -R2.85/5.25/3.9/6.3 -JX-2i/2i -B+ghoneydew+tL@-1@- -c1,1
  plot_one -Ey -N1

  plot_one -Er -N1 WSne -Xa1.2i -Ya01i
  plot_one -Eo -N1 Wsne -Xa1.2i -Ya3.2i
  plot_one -Ex -N1 Wsne -Xa1.2i -Ya5.4i
  #L2
  plot_one -Er -N2 wSne -Xa3.3i -Ya1i
  plot_one -Eo -N2 wsne -Xa3.3i -Ya3.2i
  plot_one -Ex -N2 wsne -Xa3.3i -Ya5.4i
  plot_one -Ey -N2 wsne+tL@-2@- -Xa3.3i -Ya7.6i
  #LMS
  plot_one -Er -Nr weSn -Xa5.4i -Ya1i >>$ps
  plot_one -Eo -Nr wesn -Xa5.4i -Ya3.2i
  plot_one -Ex -Nr wesn -Xa5.4i -Ya5.4i
  plot_one -Ey -Nr wesn+tLMS -Xa5.4i -Ya7.6i
  # Labels
echo REDUCED MAJOR AXIS | gmt pstext -R -J -F+cRM+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya1i
echo ORTHOGONAL | gmt pstext -R -J -F+cRM+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya3.2i
echo X ON Y | gmt pstext -R -J -F+cRM+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya5.4i
echo Y ON X | gmt pstext -R -J -F+cRM+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya7.6i
gmt psxy -R -J -O -T
rm -f tmp data giants
