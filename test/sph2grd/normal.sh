#!/usr/bin/env bash
#
# Check normalization; see
#	1. http://gmt.soest.hawaii.edu/issues/1127
#	2. https://github.com/GenericMappingTools/gmt/pull/4939
# Checking that the surface integral of inner product is ~ 1

echo 10 5 1 0 > t.txt
gmt sph2grd t.txt -Rg -I1 -Nm -Gsh.grd
gmt grdmath sh.grd 2 POW 90 Y SUB SIND XINC D2R 2 POW MUL MUL SUM = t1.grd
A1=$(gmt grdinfo t1.grd -Cn -o4)
gmt grdmath -Rg -I1 10 5 YLM = sh.grd POP
gmt grdmath sh.grd 2 POW 90 Y SUB SIND XINC D2R 2 POW MUL MUL SUM = t2.grd
A2=$(gmt grdinfo t2.grd -Cn -o4)
rm -f fail
if [ $(gmt math -Q $A1 1 SUB 0.00556 GT =) -ne 0 ]; then
	echo $A1 >> fail
fi
if [ $(gmt math -Q $A2 1 SUB 0.00556 GT =) -ne 0 ]; then
	echo $A2 >> fail
fi
