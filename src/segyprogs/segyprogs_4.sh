#!/bin/sh
#
#	$Id: segyprogs_4.sh,v 1.1 2002-09-27 19:15:03 pwessel Exp $
#
# script to plot mendo wa1 combined data
#
# cdp = 40 * coordinate on line, 30km max depth
#

area1=-R-35/6/0/30
proj1="-Jx0.15i/-0.15i"
outfile=segyprogs_4.ps

gmtset COLOR_FOREGROUND 255/0/0 COLOR_BACKGROUND 0/0/255
makecpt -T-5/5/1 -Z -Cpolar > test.cpt
psbasemap $area1 $proj1 -Bf5a10 -Z0.001 -Y1.5i -K -X1.5i > $outfile
segy2grd $area1  -I0.1/0.1 wa1_mig13.segy -Gtest.grd -V
grdimage $area1 $proj1 -O test.grd -Ctest.cpt >> $outfile
gmtset COLOR_FOREGROUND 255/255/255 COLOR_BACKGROUND 0/0/0
rm -f test.cpt
