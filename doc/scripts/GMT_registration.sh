#!/bin/bash
#	$Id: GMT_registration.sh,v 1.5 2011-06-22 00:35:07 remko Exp $
#
. ./functions.sh

# Gridline registration
psxy -R0/3/0/3 -JX2.5i/1.25i -B1gwesn -P -K -Wthinner -L -Glightred << EOF > GMT_registration.ps
0.5	1.5
1.5	1.5
1.5	2.5
0.5	2.5
EOF
grdmath -R0/3/0/3 -I1 0 = $$.nc
grd2xyz $$.nc | psxy -R0/3/0/3 -JX2.5i/1.25i -Sc0.12i -N -G0 -O -K >> GMT_registration.ps

# Pixel registration
psxy -R0/3/0/3 -JX2.5i/1.25i -B1gwesn -O -K -W0p -L -Glightred -X2.75i << EOF >> GMT_registration.ps
1	1
2	1
2	2
1	2
EOF
grdmath -R0/3/0/3 -I1 -r 0 = $$.nc
grd2xyz $$.nc | psxy -R0/3/0/3 -JX2.5i/1.25i -Sc0.12i -Gblack -O >> GMT_registration.ps
rm -f $$.*
