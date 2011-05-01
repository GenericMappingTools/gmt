#!/bin/bash
#	$Id: GMT_registration.sh,v 1.3 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

# Gridline registration
psxy -R0/3/0/3 -JX2.5i/1.25i -B1g1wesn -P -K -Wthinner -L -Glightred << EOF > GMT_registration.ps
0.5	1.5
1.5	1.5
1.5	2.5
0.5	2.5
EOF
xyz2grd -R0/3/0/3 -I1 -N0 /dev/null -G$$.nc 1>/dev/null 2>&1
grd2xyz $$.nc | psxy -R0/3/0/3 -JX2.5i/1.25i -Sc0.12i -N -G0 -O -K >> GMT_registration.ps

# Pixel registration
psxy -R0/3/0/3 -JX2.5i/1.25i -B1g1wesn -O -K -W0p -L -Glightred -X2.75i << EOF >> GMT_registration.ps
1	1
2	1
2	2
1	2
EOF
xyz2grd -R0/3/0/3 -I1 -r -N0 /dev/null -G$$.nc 2>&1 1>/dev/null
grd2xyz $$.nc | psxy -R0/3/0/3 -JX2.5i/1.25i -Sc0.12i -Gblack -O >> GMT_registration.ps
rm -f $$.*
