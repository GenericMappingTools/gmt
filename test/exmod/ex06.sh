#!/bin/bash
#		GMT EXAMPLE 06
#		$Id$
#
# Purpose:	Make standard and polar histograms
# GMT modules:	histogram, rose
#
gmt begin ex06 ps
  gmt set PS_MEDIA letter
  gmt subplot begin 2x1 -A+JTL+o0.3i -Fs6i/3.5i -M0.4i -X1.5i -Y1.5i
    gmt rose @fractures_06.txt -: -A10r -Sn -JX3.6i -Gorange -R0/1/0/360 -Bx0.2g0.2 -By30g30 -B+glightblue -W1p -c2,1
    gmt histogram -Bx+l"Topography (m)" -By+l"Frequency"+u" %" -BWSne+t"Histograms"+glightblue @v3206_06.txt \
	-R-6000/0/0/30 -JX4.8i/2.4i -Gorange -L1p -Z1 -W250 -c1,1
  gmt subplot end
gmt end
