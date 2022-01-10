#!/usr/bin/env bash
#
# Test the C API for reading via a matrix and making a plot
# DVC_TEST
ps=apicubeplot.ps
if [ ! -f cube.nc ]; then
	cp "${src}"/../grdinterpolate/cube.nc .
fi
testapi_cube 
gmt grdcontour newlayer.nc -JX15c -C5 -A10 -Baf -P > $ps
