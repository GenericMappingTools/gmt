#!/usr/bin/env bash
# Test grdinterpolate slicing along equator through a 3-D grid
# Getting the file directly from IRIS

gmt begin slices ps
	gmt set PROJ_ELLIPSOID sphere
	curl -q http://ds.iris.edu/spudservice/data/17996723 -o S362ANI_kmps.nc
	gmt grdinterpolate S362ANI_kmps.nc?vsv -E0/0/180/0+i0.25d+g+p -T25/2890/5 -Gvsv.nc
	gmt grdinterpolate S362ANI_kmps.nc?vsh -E0/0/180/0+i0.25d+g+p -T25/2890/5 -Gvsh.nc
	gmt grdinterpolate S362ANI_kmps.nc?vs -E0/0/180/0+i0.25d+g+p -T25/2890/5 -Gvs.nc
	gmt makecpt -Cseis -T4.25/7.35
	gmt subplot begin 4x2 -Fs8c/5c -Bafg -R0/180/0/2900 -X1.5c -A
		gmt grdimage vs.nc -JP?+fp -c  -BWESn
		gmt grdimage vs.nc -JP?+fp+a+t90 -c -BWESn
		gmt grdimage vsv.nc -JP?+fp -c  -BWESn
		gmt grdimage vsv.nc -JP?+fp+a+t90 -c -BWESn
		gmt grdimage vsv.nc -JP?+fp+zp+a+t90 -c -BWESn
		gmt grdimage vsh.nc -JP?+fp -c  -BWESn
		gmt grdimage vsh.nc -JP?+fp+a+t90 -c -BWESn
		gmt grdimage vsh.nc -JP?+fp+zp+a+t90 -c -BWESn
	gmt subplot end
gmt end show
