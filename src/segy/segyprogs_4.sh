#!/bin/bash
#
#
# script to plot mendo wa1 combined data
#
# cdp = 40 * coordinate on line, 30km max depth
#

ps=segyprogs_4.ps
area1=-R-35/6/0/30
proj1="-Jx0.15i/-0.15i"

makecpt -T-5/5 -Z -Cpolar > test.cpt
gmt segy2grd $area1 -I0.1/0.1 wa1_mig13.segy -Gtest.nc -V
gmt grdimage $area1 $proj1 -K test.nc -Ctest.cpt -Xc -P > $ps
gmt psbasemap -R -J -Baf -O  >> $ps
rm -f test.nc test.cpt
