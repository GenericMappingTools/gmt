#!/usr/bin/env bash
# Test subplot with two pre-loaded images

gmt begin subplot_images ps
	gmt grdmath -R0/10/0/10 -I0.1 X Y MUL = grid1.grd
	gmt grdmath -R0/10/0/10 -I0.1 X Y ADD = grid2.grd
	
	gmt grdimage grid1.grd -Aimg1.tif -Cjet -JX10c/10c -Q
	gmt grdimage grid2.grd -Aimg2.tif -Cjet -JX10c/10c -Q
	
	gmt subplot begin 2x1 -Fs10c/10c -A+gwhite+p0.5p
		gmt subplot set 0 -A"Image 1"
		gmt grdimage img1.tif -Cjet
		gmt subplot set 1 -A"Image 2"
		gmt grdimage img2.tif -Cjet
	gmt subplot end
	rm -f grid1.grd grid2.grd img1.tif img2.tif
gmt end show
