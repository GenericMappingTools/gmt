#!/bin/bash
#	$Id: GMT_App_B_2.sh,v 1.6 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

trap 'rm -f $$.*; exit 1' 1 2 3 15

psxy -R0/3/0/3 -JX2.5i/1.25i -B1g1wesn -P -K -W0p -L -Ggray << EOF > GMT_App_B_2.ps
1	1
2	1
2	2
1	2
EOF
xyz2grd -R0/3/0/3 -I1 -F -N0 /dev/null -G$$.nc 2>&1 1>/dev/null
grd2xyz $$.nc | psxy -R0/3/0/3 -JX2.5i/1.25i -Sc0.12i -Gblack -O >> GMT_App_B_2.ps
rm -f $$.*
