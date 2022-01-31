#!/usr/bin/env bash
#
# Test using the cumulative density function to create an equal-area
# CPT for a chunk of the word.
# DVC_TEST

gmt begin equalarea ps
	gmt set MAP_FRAME_TYPE plain
	gmt grdcut -R0/90/0/45 @earth_relief_05m -Gtmp.grd
	gmt subplot begin 2x1 -Fs16c/11c -BWSrt -M6p -T"CPT Equalization" -Y0.5i
	gmt subplot set 0
	gmt grd2cpt tmp.grd -E11
	gmt grdimage tmp.grd -JM?
	gmt colorbar -DJTC -Baf
	gmt subplot set 1
	gmt grd2cpt tmp.grd -E11+c+fcdf.txt
	gmt grdimage tmp.grd -JM?
	gmt colorbar -DJTC -Baf
	gmt subplot end
gmt end show
