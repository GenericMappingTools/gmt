#!/bin/bash
#	$Id$

# script to plot mendo wa1 combined data
#
# cdp = 40 * coordinate on line, 30km max depth
#
area1=-R-35/6/-35/6/0/30
proj1="-Jx0.15i/-0.15i -Jz-0.15i -E175/05"
outfile=segyprogs_3.ps

gmt psbasemap $area1 $proj1 -Bxf5a10+lX -Byf5a10+lY -Bzf5a10 -BWSneZ -Z0.001 -Y1.5i -K -X1.5i > $outfile
gmt gmt pssegyz wa1_mig13.segy -R $proj1 -X0.1i -D0.35 -C8.0 -Y0.1i -W -Fgreen -B-1.2 -Sc/5 -O -K >> $outfile
gmt gmt pssegyz wa1_mig13.segy -R $proj1 -X0.1i -D0/0.35 -C8.0 -Y0.1i -Fred -B-1.2 -S5/c -O >> $outfile
