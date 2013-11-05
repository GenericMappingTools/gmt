#!/bin/sh
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

psbasemap $area1 $proj1 -Bf5a10 -Z0.001 -Y1.5i -K -X1.5i > $outfile
pssegy wa1_mig13.segy -R -Jx -X0.1i -D0.35 -C8.0 -Y0.1i -W -F0/255/0 -B-1.2 -Sc -O >> $outfile 
