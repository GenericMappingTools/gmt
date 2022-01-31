#!/usr/bin/env bash
# Create 8 layers of Cartesian data, faking it
# The original PS was made with the commented out lines so that we
# can be sure the stacking into a cube and pulling out layers work
# DVC_TEST
gmt begin fake_xyz_cube ps
	gmt math -T1/8/1 -o0 --FORMAT_FLOAT_OUT=%01.0f T = z.txt
	while read z; do
		gmt grdmath -R-10/10/-10/10 -I1 0 0 CDIST $z 0.25 MUL HYPOT $z DIV 4 POW -0.5 MUL EXP 5 MUL 8 $z SUB ADD = tmp_$z.grd
	done < z.txt
	gmt grdinterpolate -Zz.txt tmp_?.grd -Gfake_xyz_cube.nc
	gmt makecpt -T0/12
	gmt subplot begin 4x2 -Fs5c -Scb -Srl -Blrbt -A1+gwhite -X5c -Y4c -M1c/0.25c
		for z in $(seq 1 8); do
			#gmt grdimage "tmp_${z}.grd" -c
			gmt grdimage "fake_xyz_cube.nc?(${z})" -c
		done
	gmt subplot end
	gmt colorbar -DJBC -B
gmt end show
#rm -f tmp_?.grd
