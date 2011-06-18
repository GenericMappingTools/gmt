#!/bin/bash
#	$Id: trend_linear.sh,v 1.1 2011-06-18 16:50:50 jluis Exp $
#
# Remove a bilinear trend

. ../functions.sh
header "Test grdtrend to remove a bilinear trend"

grdmath -R-15/15/-15/15 -I0.1 X Y ADD = lixo1.grd
grdtrend lixo1.grd -N3 -Dlixo2.grd
grdinfo lixo2.grd -C -L2 | gmtconvert -o11 > lixo1.dat
echo 0 > lixo2.dat

diff lixo1.dat lixo1.dat --strip-trailing-cr > fail

rm -f lixo*

passfail trend_linear
