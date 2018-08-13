#!/bin/bash

# script to plot mendo wa1 combined data
#
# cdp = 40 * coordinate on line, 30km max depth
#
ps=segy_03.ps
area1=-R-35/6/-35/6/0/30
proj1="-Jx0.15i/-0.15i -Jz-0.15i -p175/05"

gmt pssegyz "${src:-.}"/wa1_mig13.segy $area1 $proj1 -Qx0.1 -D0.35 -C8.0 -Qy0.1 -W -Fgreen -Qb-1.2 -Sc/5 -P -K -Xc > $ps
gmt pssegyz "${src:-.}"/wa1_mig13.segy -R $proj1 -Qx0.1 -D0/0.35 -C8.0 -Qy0.1 -Fred -Qb-1.2 -S5/c -O -K >> $ps
gmt psbasemap -R $proj1 -Bxf5a10+lX -Byf5a10+lY -Bzf5a10 -BWSneZ -O >> $ps
