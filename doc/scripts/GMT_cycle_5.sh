#!/usr/bin/env bash
# Demonstration of -w through a series of cycle plots
# This one shows how to wrap data and grid and make a quick image
gmt begin GMT_cycle_5
	gmt set GMT_THEME cookbook
	gmt xyz2grd @mississippi.txt -i0,1+s1e-3,1+s1e-3 -wy -R0/1/0/50 -I50+n -r -Gtmp.grd
 	gmt grdimage tmp.grd -JX15c/8c -BWSen -Bxaf+l"Normalized year" -Byaf+l"Discharge (10@+3@+ m@+3@+/s)"
 	gmt colorbar -DJRM -Baf
gmt end show
