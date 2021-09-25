#!/usr/bin/env bash
# Test grdmix by deconstructing an image into r,g,b grids the
# reassembly to another image.

gmt begin mixcompose
	gmt set MAP_FRAME_TYPE plain PS_MEDIA letter
	gmt grdmix @earth_day_01d -D -Glayer_%c.grd
	gmt grdmix layer_R.grd layer_G.grd layer_B.grd -Ni -C -Gearth_day.tif
	gmt grdimage earth_day.tif -JQ18c -B
	gmt subplot begin 2x2 -Fs8.5c/4.25c -JQ8.5c -Rd -Srl -Scb -Y10c -M10p/4p -T"Deconstruct/construct Earth Day"
		gmt grdimage @earth_day_01d -c
		gmt makecpt -T0/255 -Cwhite,red -H > t.cpt
		gmt grdimage layer_R.grd -Ct.cpt -c
		gmt makecpt -T0/255 -Cwhite,green -H > t.cpt
		gmt grdimage layer_G.grd -Ct.cpt -c
		gmt makecpt -T0/255 -Cwhite,blue -H > t.cpt
		gmt grdimage layer_B.grd -Ct.cpt -c
	gmt subplot end
gmt end show
