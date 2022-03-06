#!/usr/bin/env bash
#	Test the use of a 2nd data grid in -I to derive intensities from
#	even though that grid has a different dimension than data grid

gmt begin twogrids
	# Get the data grid that will provide the shading
	gmt grdcut -R180/190/10/20 @earth_relief_06m -Gz.grd
	# Set up a cpt for crustal ages 100-150 Ma
	gmt makecpt -Croma -T100/150/5
	gmt subplot begin 3x1 -Fs8c -Baf -R180/190/10/20 -JM7.5c -A+gwhite+p0.25p+o0.2c+jTC -Y1.5c
		gmt subplot set -A"GRD2 same resolution [6m]"
		gmt grdcut -R180/190/10/20 @earth_age_06m -Gt.grd
		gmt grdimage t.grd -Iz.grd+d
		gmt subplot set -A"GRD2 courser resolution [10m]"
		gmt grdcut -R180/190/10/20 @earth_age_10m -Gt.grd
		gmt grdimage t.grd -Iz.grd+d
		gmt subplot set -A"GRD2 finer resolution [2m]"
		gmt grdcut -R180/190/10/20 @earth_age_02m -Gt.grd
		gmt grdimage t.grd -Iz.grd+d
	gmt subplot end
gmt end show
