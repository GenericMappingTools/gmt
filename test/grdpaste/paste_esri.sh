#!/bin/bash
#	$Id$
#
# Paste esri ASCII grids along Y

# The final grid is just f(x,y) = x
gmt grdmath -R-15/15/-15/0 -I0.5 X = lixo_y1.asc=ef
gmt grdmath -R-15/15/0/15 -I0.5 X = lixo_y2.asc=ef

gmt grdpaste lixo_y1.asc lixo_y2.asc -Glixo_y.nc
gmt grdmath -R-15/15/-15/15 -I0.5 X = answer.nc
gmt grdmath lixo_y.nc answer.nc SUB 0 EQ = tmp.nc
n=`gmt grd2xyz tmp.nc -Z | uniq | wc -l | $AWK '{print $1}'`
if [ $n -gt 1 ]; then
	echo "Found $n different results instead of just 1" > fail
else
	touch fail
fi
