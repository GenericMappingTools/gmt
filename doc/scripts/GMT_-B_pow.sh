#!/bin/sh
#	$Id: GMT_-B_pow.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

gmtset GRID_PEN 0.25top
psbasemap -R0/100/0/0.9 -JX3p0.5/0.25 -Ba3f2g1p:"Axis Label":S -K -P > GMT_-B_pow.ps
psbasemap -R -JX -Ba20f10g5:"Axis Label":S -O -Y0.85 >> GMT_-B_pow.ps
#
# Mess because the 10^x anotations sticks outside but gets clipped
#
sed '/%%Trailer/q' GMT_-B_pow.ps > $$.ps
cat << EOF >> $$.ps
%%BoundingBox: 60 0 300 155
S 0 -255 T 4.16667 4.16667 scale showpage

end
EOF
mv -f $$.ps GMT_-B_pow.ps
gmtset GRID_PEN 0.25p
