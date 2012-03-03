#!/bin/sh
#
#	$Id$
#
# Use do identify which direction this polygon follows

R=`minmax -I0.02 $1`
psxy $R -JM6.6i+ -P -B15mg5m -Wthin $1 -K > t.ps
first=`sed -n 1p $1`
second=`sed -n ${2}p $1`
echo $first $second | psxy -R -J -O -Svs0.01/0.05/0.04 -Gred >> t.ps
gv t.ps &
