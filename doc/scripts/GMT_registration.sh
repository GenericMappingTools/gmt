#!/usr/bin/env bash
gmt begin GMT_registration
# Gridline registration
gmt plot -R0/3/0/3 -JX2.5i/1.25i -B1g1 -Bwesn -Wthinner -L -Glightred << EOF
0.5	1.5
1.5	1.5
1.5	2.5
0.5	2.5
EOF
gmt grdmath -R0/3/0/3 -I1 0 = tt.nc
gmt grd2xyz tt.nc | gmt plot -Sc0.12i -N -G0

# Pixel registration
gmt plot -B1g1 -Bwesn -W0p -L -Glightred -X2.75i << EOF
1	1
2	1
2	2
1	2
EOF
gmt grdmath -R0/3/0/3 -I1 -r 0 = tt.nc
gmt grd2xyz tt.nc | gmt plot -Sc0.12i -Gblack
rm tt.nc
gmt end show
