#!/usr/bin/env bash
# Test grdinterpolate slicing along equator through a 3-D grid
# Getting the file directly from IRIS.
# This script tests ability to supply our own profile file

gmt begin slices_file ps
	gmt set PROJ_ELLIPSOID sphere
	curl -s http://ds.iris.edu/spudservice/data/17996723 -o S362ANI_kmps.nc
	gmt math -T0/180/0.5 0 = line.txt
	gmt grdinterpolate S362ANI_kmps.nc?vs -Eline.txt -T30/2890/10 -Gvs.nc
	gmt makecpt -Cseis -T4.25/7.35
	gmt subplot begin 3x1 -Fs12c/7c -Bafg -R0/180/0/2900  -A
		gmt grdimage vs.nc -JP?+fp+kx -c  -BWESn
		gmt grdimage vs.nc -JP?+fp+a+t90 -c -BWESn
		gmt grdimage vs.nc -JP?+fp+zp+a+t90 -c -BWESn
	gmt subplot end
gmt end show
