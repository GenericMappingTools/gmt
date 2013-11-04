#!/bin/bash
#	$Id$
#
# Gridline registration
gmt psxy -R0/3/0/3 -JX2.5i/1.25i -B1g1 -Bwesn -P -K -Wthinner -L -Glightred << EOF > GMT_registration.ps
0.5	1.5
1.5	1.5
1.5	2.5
0.5	2.5
EOF
gmt grdmath -R0/3/0/3 -I1 0 = tt.nc
gmt grd2xyz tt.nc | gmt psxy -R0/3/0/3 -JX2.5i/1.25i -Sc0.12i -N -G0 -O -K >> GMT_registration.ps

# Pixel registration
gmt psxy -R0/3/0/3 -JX2.5i/1.25i -B1g1 -Bwesn -O -K -W0p -L -Glightred -X2.75i << EOF >> GMT_registration.ps
1	1
2	1
2	2
1	2
EOF
gmt grdmath -R0/3/0/3 -I1 -r 0 = tt.nc
gmt grd2xyz tt.nc | gmt psxy -R0/3/0/3 -JX2.5i/1.25i -Sc0.12i -Gblack -O >> GMT_registration.ps
