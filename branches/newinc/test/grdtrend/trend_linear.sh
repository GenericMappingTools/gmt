#!/bin/bash
#	$Id$
#
# Remove a bilinear trend

gmt grdmath -R-15/15/-15/15 -I0.1 X Y ADD = lixo1.grd
gmt grdtrend lixo1.grd -N3 -Dlixo2.grd
gmt grdinfo lixo2.grd -C -L2 | gmt gmtconvert -o11 > lixo1.dat
echo 0 > lixo2.dat

diff lixo1.dat lixo1.dat --strip-trailing-cr > fail
