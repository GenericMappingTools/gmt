#!/bin/bash
#	$Id$
#
# Paste esri ascii grids along Y

header "Test grdpaste with ESRI ASCII files"

# The final grid is just f(x,y) = x
grdmath -R-15/15/-15/0 -I0.5 X = lixo_y1.asc=ef
grdmath -R-15/15/0/15 -I0.5 X = lixo_y2.asc=ef

grdpaste lixo_y1.asc lixo_y2.asc -Glixo_y.nc
grdmath -R-15/15/-15/15 -I0.5 X = answer.nc
grdmath lixo_y.nc answer.nc SUB 0 EQ = tmp.nc
n=`grd2xyz tmp.nc -Z | uniq | wc -l | awk '{print $1}'`
if [ $n -gt 1 ]; then
	echo "Found $n different results instead of just 1" > fail
fi

passfail paste_esri
