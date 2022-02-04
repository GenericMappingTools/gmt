#!/usr/bin/env bash
# Ensure that grdimage picks up the CPT stored in the netCDF z:cpt attribute
# https://github.com/GenericMappingTools/gmt/issues/6167
# Fixed via 
# DVC_TEST
gmt begin defcpt
	gmt grdcut @earth_relief_01d_g -R0/10/0/10 -Gtest.nc
	gmt subplot begin 2x1 -R0/10/0/10 -JQ10c -Fs10c
		gmt grdimage @earth_relief_01d_g -R0/10/0/10 -B0 -c
		gmt grdimage test.nc -B0 -c
	gmt subplot end
gmt end show
