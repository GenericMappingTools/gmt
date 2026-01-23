#!/usr/bin/env bash
#
# Test for Issue #8053: nodal planes plotting incorrectly for vertical faults
# Key test cases: dip=90° (vertical) and rake=180° trigger NaN in projection
#

gmt begin psmeca_radius png
	gmt basemap -R0/12/0/6 -JM20c -B1 -BWSen

	echo 3 4.5 0 30 75 180 5 S30 D75 R180 | gmt meca -Sa3c -N -L -T

	echo 9 4.5 10 0 90 0 5 S0 D90 R0 | gmt meca -Sa3c -N -L -T

	echo 3 1.5 10 80 90 -140 5 S80 D90 R-140 | gmt meca -Sa3c -N -L -T

	echo 9 1.5 10 0 90 180 5 S0 D90 R180 | gmt meca -Sa3c -N -L -T

gmt end
