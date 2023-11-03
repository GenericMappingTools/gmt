#!/usr/bin/env bash
# Test grdmix by deconstructing an image into r,g,b grids the
# reassembly to another image.

gmt begin GMT_mixing
	gmt set MAP_FRAME_TYPE plain FONT_ANNOT_PRIMARY 7p,Helvetica,black
	gmt grdgradient @earth_relief_20m_p -A45 -Nt1 -Gint.grd
	gmt grdimage @earth_day_20m_p -JQ9c -B0 -Iint.grd
	gmt grdmix @earth_day_01d -D -Glayer_%c.grd
	gmt subplot begin 2x2 -Fs4.2c/2.125c -JQ4.25c -Rd -Srr -Scb -B0 -M1p/0.5p -X9.3c
		gmt grdimage int.grd -Cgray -c
		gmt makecpt -T0/255 -Cwhite,red -H > t.cpt
		gmt grdimage layer_R.grd -Ct.cpt -c
		gmt makecpt -T0/255 -Cwhite,green -H > t.cpt
		gmt grdimage layer_G.grd -Ct.cpt -c
		gmt makecpt -T0/255 -Cwhite,blue -H > t.cpt
		gmt grdimage layer_B.grd -Ct.cpt -c
	gmt subplot end
gmt end show
