#!/bin/sh 
#	$Id: GMT_App_B_1.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

psxy -R0/3/0/3 -JX2.5i/1.25i -B1g1wesn -P -K -W0.5p -L -G200 << EOF > GMT_gridreg.ps
0.5	1.5
1.5	1.5
1.5	2.5
0.5	2.5
EOF
xyz2grd -R0/3/0/3 -I1 -N0 /dev/null -Gt.grd 1>/dev/null 2>&1
grd2xyz t.grd | psxy -R0/3/0/3 -JX2.5i/1.25i -Sc0.12i -N -G0 -O >> GMT_gridreg.ps
\rm -f t.grd
