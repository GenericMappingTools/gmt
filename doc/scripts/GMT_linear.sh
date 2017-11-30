#!/bin/bash
#	$Id$
#
gmt psxy -R0/100/0/10 -JX3i/1.5i -Bag -BWSne+gsnow -Wthick,blue,- -P -K sqrt.txt > GMT_linear.ps
gmt psxy -R -J -St0.1i -N -Gred -Wfaint -O sqrt10.txt >> GMT_linear.ps
