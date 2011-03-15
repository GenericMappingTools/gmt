#!/bin/sh
#
#	$Id: segyprogs_4.sh,v 1.5 2011-03-15 02:06:37 guru Exp $
#
# script to plot mendo wa1 combined data
#
# cdp = 40 * coordinate on line, 30km max depth
#

area1=-R-35/6/0/30
proj1="-Jx0.15i/-0.15i"
outfile=segyprogs_4.ps

psbasemap $area1 $proj1 -Bf5a10 -Z0.001 -Y1.5i -K -X1.5i > $outfile
./segy2grd $area1  -I0.1/0.1 wa1_mig13.segy -Gtest.nc -V
grdimage $area1 $proj1 -O test.nc -Ctest.cpt >> $outfile
rm -f test.nc
