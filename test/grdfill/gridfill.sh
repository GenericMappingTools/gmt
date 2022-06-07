#!/usr/bin/env bash
# Test the -Ag algorithm in grdfill
# The commented step are the commands making the file now in the cache
# Pull out a small piece of 20m DEM
# gmt grdcut -R0/10/0/10 @earth_relief_20m_g -Gdata.grd
# Make a mask for two holes
# gmt grdmath -Rdata.grd 4 6 SDIST 250 GE 0 NAN 7 1.5 SDIST 100 GE 0 NAN ADD 2 DIV = mask.grd
# Combine to get data with the holes
# gmt grdmath data.grd mask.grd MUL = earth_relief_20m_holes.grd
# The file earth_relief_20m_holes.grd is stored in the cache
# Try filling in the holes via sampling from a coarser 1d grid
gmt begin gridfill
	gmt makecpt -Cgeo
	gmt grdfill @earth_relief_20m_holes.grd -Ag@earth_relief_01d -Gnew.grd
	gmt subplot begin 2x1 -R0/10/0/10 -JQ10c -Fs10c -Sc -Sr -A1+gwhite+r
		gmt grdimage @earth_relief_20m_holes.grd -c
		gmt grdimage new.grd -c
	gmt subplot end
	gmt colorbar -Baf -DJBC
gmt end show
