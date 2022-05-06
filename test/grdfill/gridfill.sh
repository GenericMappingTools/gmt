#!/usr/bin/env bash
# Test the -Ag algorithm in grdfill
# Pull out a small piece of 6m DEM
gmt grdcut -R0/10/0/10 @earth_relief_06m -Gdata.grd
# Make a mask for two holes
gmt grdmath -Rdata.grd 4 6 SDIST 250 GE 0 NAN 7 1.5 SDIST 100 GE 0 NAN ADD 2 DIV = mask.grd
# Combine to get data with the holes
gmt grdmath data.grd mask.grd MUL = holes.grd
# Try filling in the holes via sampling from a coarser 30m grid
gmt grdfill holes.grd -Ag@earth_relief_30m -Gnew.grd
# Compute difference with original
gmt grdmath data.grd new.grd SUB = diff.grd
gmt begin gridfill
	gmt makecpt -Cgeo
	gmt subplot begin 3x2 -R0/10/0/10 -JQ6c -Fs6c -Sc -Sr -A1+gwhite+r
		gmt grdimage data.grd -c
		gmt grdimage mask.grd -c
		gmt grdimage holes.grd -c
		gmt grdimage holes.grd -Q -c
		gmt grdimage new.grd -c
		gmt grdimage diff.grd -c
	gmt subplot end
	gmt colorbar -Baf -DJBC
gmt end show
