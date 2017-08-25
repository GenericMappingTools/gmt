#!/bin/bash
#               GMT EXAMPLE 47
#               $Id$
#
# Purpose:      Illustrate use of gmtregress with different norms and types
# GMT modules:  gmtregress, psxy
#

export GMT_PPID=1
function plot_one { # First four args are: -E -N -c -Barg
  gmt regress data -Fxm $1 $2 -T2.85/5.25/0.1 > tmp
  gmt psxy -R2.85/5.25/3.9/6.3 -B+ghoneydew${4} data -Sc0.05i -Gblue $3
  gmt psxy giants -Sc0.05i -Gred   -N
  gmt psxy giants -Sc0.1i  -W0.25p -N
  gmt psxy -W2p tmp	
}

gmt begin ex47 ps
  gmt which -Gl @hertzsprung-russell.txt
  # Allow outliers (commented out by #) to be included in the analysis:
  sed -e s/#//g hertzsprung-russell.txt > data
  # Identify the red giants (outliers)
  grep '#' hertzsprung-russell.txt | sed -e s/#//g > giants
  gmt subplot begin 4x3 -Dx -M1p -Fs2i/2i -LRl+l"Log light intensity" -LCb+l"Log temperature"+tc -Lwesn+g
  # L1
  plot_one -Ey -N1 -c1,1 +tL@-1@-
  plot_one -Er -N1 -c2,1
  plot_one -Eo -N1 -c3,1
  plot_one -Ex -N1 -c4,1
  #L2
  plot_one -Er -N2 -c1,2 +tL@-2@- 
  plot_one -Eo -N2 -c2,2
  plot_one -Ex -N2 -c3,2
  plot_one -Ey -N2 -c4,2
  #LMS
  plot_one -Er -Nr -c1,3 +tLMS
  plot_one -Eo -Nr -c2,3
  plot_one -Ex -Nr -c3,3
  plot_one -Ey -Nr -c4,3
gmt end
# Labels
#echo REDUCED MAJOR AXIS | gmt pstext -R -J -F+cRM+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya1i
#echo ORTHOGONAL | gmt pstext -R -J -F+cRM+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya3.2i
#echo X ON Y | gmt pstext -R -J -F+cRM+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya5.4i
#echo Y ON X | gmt pstext -R -J -F+cRM+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya7.6i
#gmt psxy -R -J -O -T
rm -f tmp data giants
