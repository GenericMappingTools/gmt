#!/bin/sh
#
#	$Id: view.sh,v 1.1 2004-09-05 04:25:01 pwessel Exp $
#
# Use to zoom in on area near a particular point in a polygon
# view.sh polid recno [dx]

file="polygon.$1"
p=$2
if [ $# -ne 3 ]; then
	d=0.02
else
	d=$3
fi
lon=`sed -n ${p}p $file | cut -f1`
lat=`sed -n ${p}p $file | cut -f2`
w=`gmtmath -Q $lon $d SUB =`
e=`gmtmath -Q $lon $d ADD =`
s=`gmtmath -Q $lat $d SUB =`
n=`gmtmath -Q $lat $d ADD =`
psxy -R$w/$e/$s/$n -JM7i+ -P -B2m $file -Wthin -K -X0.75i > t.ps
psxy -R -J -O $file -Sc0.025 -Gred >> t.ps
gv t.ps &
