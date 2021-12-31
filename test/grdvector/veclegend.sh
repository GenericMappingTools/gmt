#!/usr/bin/env bash
# Demonstrate the auto-legend entry for grdvector
# Make a fake data set of data vectors in mm/yr
gmt grdmath -R0/30/30/60 -I5 X = x.grd
gmt grdmath -R0/30/30/60 -I5 Y 30 SUB = y.grd
# Set the LL values to the same as the UR values
echo 0 30 30 > t.txt
gmt grdedit -Nt.txt x.grd
gmt grdedit -Nt.txt y.grd
# use reference length as 30 mm/year in the legend
gmt begin veclegend
	gmt grdvector x.grd y.grd -R-2/38/29/65 -JM15c -B -Q14p+e+n100k/0 -Gred -W1p -S15c+r30 -l"Velocity (30 mm/yr)"
	echo 13 47 | gmt plot -Sc0.1i -Gblue -l"Arbitrary point"
gmt end show
