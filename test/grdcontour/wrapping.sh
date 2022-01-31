#!/usr/bin/env bash
# Creating a periodic 0/360 grid then selecting a central meridian for a contour map
# The central meridian is not one of the grid nodes.
# https://github.com/GenericMappingTools/gmt/issues/6066
# DVC_TEST
gmt begin wrapping
	gmt grdmath -Rg -I10 35 21 SDIST = tmp_dist.nc
	gmt grdcontour -Rd -JW35/15c tmp_dist.nc -C2500 -Wthin -B0
gmt end show
