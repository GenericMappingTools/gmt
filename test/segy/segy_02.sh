#!/bin/bash
#
#
# script to plot mendo wa1 combined data
#
# cdp = 40 * coordinate on line, 30km max depth
#
ps=segy_02.ps

area1=-R-35/6/0/30
proj1="-Jx0.15i/-0.15i"

gmt pssegy "${src:-.}"/wa1_mig13.segy $area1 $proj1 -Qx0.1 -D0.35 -C8.0 -Qy0.1 -W -Fgreen -Qb-1.2 -Sc -K -T"${src:-.}"/test.list -E0.001 -Z -P -Xc > $ps
gmt psbasemap -R -J -Baf -O >> $ps
