#!/usr/bin/env bash
#
# # Check psmeca for cases where P and T axes have equal dips (e.g., vertical dip-slip)
gmt begin seis_equal_dip ps
gmt basemap -R0/10/0/6.5 -JM15c -B0 -BWSen+t"Focal Mechanisms Comparison"

echo 3.0  5.0 10.0 0 90 90 5.0 0 0 Vertical dip-slip | gmt meca -Sa3.2c+m -Gred -W0.8p,black
echo 7.0  5.0 10.0 0 90 0  5.0 0 0 Strike-slip | gmt meca -Sa3.2c+m -Gblue -W0.8p,black

echo 3.0  1.8 5   1 360 45 1 90 0 1 180 45   1 Dip=45 | gmt meca -Sy3.2c+m -Gpurple   -W0.8p,black
echo 7.0  1.8 5   1 360 45 1 90 0 1 180 45.1 1 Dip=45.1 | gmt meca -Sy3.2c+m -Gdarkgreen -W0.8p,black
gmt end