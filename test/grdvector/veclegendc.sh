#!/usr/bin/env bash
# Demonstrate the auto-legend entry for grdvector Cartesian vectors
# Make a fake data set of data vectors in mm/yr, plot them with a reference length of 25 mm/yr
# and just plot something else (a circle) later to make sure of alignment
gmt grdmath -R0/30/30/60 -I5 -fg X = x.grd
gmt grdmath -R0/30/30/60 -I5 -fg Y 30 SUB = y.grd
# Set the LL grid values to the same as the UR values
echo 0 30 30 > t.txt
gmt grdedit -Nt.txt x.grd
gmt grdedit -Nt.txt y.grd
# Use reference length as 25 mm/year in the legend
gmt begin veclegendc
	gmt grdvector x.grd y.grd -R-2/38/29/65 -JM15c -B -Q14p+e+n25q/0 -Gred -W1p -S15c+s25 -l"Velocity (25 mm/yr)"
	echo 13 47 | gmt plot -Sc0.1i -Gblue -l"Arbitrary point"
gmt end show
