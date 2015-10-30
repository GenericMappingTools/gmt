#!/bin/bash
#
#	$Id$
#
# script to plot mendo wa1 combined data
#
# cdp = 40 * coordinate on line, 30km max depth
#

area1=-R-35/6/0/30
proj1="-Jx0.15i/-0.15i"
outfile=segyprogs_1.ps

gmt psbasemap $area1 $proj1 -Baf -Z0.001 -Y1.5i -K -X1.5i > $outfile
gmt pssegy wa1_mig13.segy -R -J -X0.1i -D0.35 -C8.0 -Y0.1i -W -Fgreed -B-1.2 -Sc -O >> $outfile 
