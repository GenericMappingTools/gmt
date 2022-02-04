#!/usr/bin/env bash
# Testing gmt grdcut -F
# DVC_TEST

gmt begin polycut
	# Get France polygon
	gmt coast -EFR -M > FR.txt
	# Crop output grid to bounding box of FR
	gmt grdcut @earth_relief_30m -FFR.txt+c -GFR_only.grd
	# Same but set inside, not outside, to NaN
	gmt grdcut @earth_relief_30m -FFR.txt+c+i -GFR_not.grd
	# Like first, but retain input region
	gmt grdcut @earth_relief_30m -FFR.txt -GFR_world.grd
	gmt grdimage FR_world.grd -B -Rd -JQ0/18c -Cturbo
	gmt subplot begin 1x2 -Fs8.5c -Scb -Srl -RFR -JM8.5c -M0.35c -Yh+1c
		gmt grdimage FR_only.grd -Cturbo -c
		gmt grdimage FR_not.grd -c
	gmt subplot end
gmt end show
