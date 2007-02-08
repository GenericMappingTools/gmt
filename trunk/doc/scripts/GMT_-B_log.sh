#!/bin/sh
#	$Id: GMT_-B_log.sh,v 1.5 2007-02-08 21:46:27 remko Exp $
#

gmtset GRID_PEN_PRIMARY thinnest,.
psbasemap -R1/1000/0/1 -JX3l/0.25 -Ba1f2g3p:"Axis Label":S -K -P > GMT_-B_log.ps
psbasemap -R -J -Ba1f2g3l:"Axis Label":S -O -K -Y0.85 >> GMT_-B_log.ps
psbasemap -R -J -Ba1f2g3:"Axis Label":S -O -Y0.85 >> GMT_-B_log.ps
#
# Mess because the 10^x annotations sticks outside but gets clipped
#
sed '/%%Trailer/q' GMT_-B_log.ps > $$.ps
cat << EOF >> $$.ps
%%BoundingBox: 60 0 300 216
S 0 -255 T 4.16667 4.16667 scale showpage

end
EOF
mv -f $$.ps GMT_-B_log.ps
