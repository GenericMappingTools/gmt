#!/bin/sh
#
#	$Id$
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
dd=`gmtmath -Q $d 2 MUL 60 MUL 5 DIV FLOOR =`
lon=`sed -n ${p}p $file | cut -f1`
lat=`sed -n ${p}p $file | cut -f2`
w=`gmtmath -Q $lon $d SUB =`
e=`gmtmath -Q $lon $d ADD =`
s=`gmtmath -Q $lat $d SUB =`
n=`gmtmath -Q $lat $d ADD =`
psxy -R$w/$e/$s/$n -JM8i+ -P -B${dd}mWSne $file -Wthin -K -X0.75i > t.ps
psxy -R -J -O -K $file -Sc0.025 -Gred >> t.ps
echo $lon $lat | psxy -R -J -O -Sc0.025 -Gblue >> t.ps
gv t.ps &
