#!/usr/bin/env bash
#
#
# script to plot mendo wa1 combined data
#
# cdp = 40 * coordinate on line, 30km max depth
#

ps=segyprogs_5.ps
area1=-R-35/6/0/30
proj1="-Jx0.15i/-0.15i"

makecpt -T-5/5 -Z -Cpolar > test.cpt
gmt segy2grd $area1 -Sc -Qx0.1 -Qy0.1 -I0.5/0.2 wa1_mig13.segy -Gtest.nc -V
gmt grdimage $area1 $proj1 -K test.nc -Ctest.cpt -P -Xc > $ps
gmt psbasemap -R -J -Baf -O >> $ps
rm -f test.nc test.cpt
