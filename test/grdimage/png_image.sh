#!/usr/bin/env bash
#
# Testing gmt grdimage with a PNG
# DVC_TEST

gmt begin png_image ps
	gmt grdmath -R0/5/0/5 -I1 -r X DUP UPPER DIV 255 MUL = r.grd
	gmt grdmath Y DUP UPPER DIV 255 MUL = g.grd
	gmt grdmath 5 X SUB 5 Y SUB MUL DUP UPPER DIV 255 MUL = b.grd
	gmt grdmix r.grd g.grd b.grd -N -C -Grgb.png
	# Lay down Cartesian image
	gmt grdimage rgb.png -JX8c/0 -B0
	# Lay down Cartesian image
	gmt grdimage rgb.png -JX-8c/-8c -B0 -X9c
	# Pretend it is geographic
	gmt grdimage rgb.png -JM8c -R0/30/35/65 -B0 -X-9c -Y10c
	gmt grdimage rgb.png -Ei -B0 -X9c
	rm -f ?.grd
gmt end show

