#!/usr/bin/env bash
#
# Test the C API for reading via a matrix and making a plot
ps=apicubeplot.ps
testapi_cube 
gmt grdcontour newlayer.nc -JX15c -C5 -A10 -Baf -P > t.ps
