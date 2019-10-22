#!/usr/bin/env bash
gmt begin GMT_tut_18
	gmt grd2cpt @tut_bathy.nc -Cocean
	gmt grdview tut_bathy.nc -JM5i -JZ2i -p135/30 -B
gmt end show
