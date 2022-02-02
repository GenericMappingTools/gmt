#!/usr/bin/env bash
# Create 8 layers of geographic data with abs time z, faking it
# The original PS was made with the commented out lines so that we
# can be sure the stacking into a cube and pulling out layers work
# DVC_TEST
gmt begin fake_geot_cube ps
	gmt set MAP_FRAME_TYPE plain PROJ_ELLIPSOID sphere TIME_UNIT d FORMAT_CLOCK_MAP=- FONT_TAG=12p
	gmt math -T2020-01-01T/2020-01-08T/1 -o0 --FORMAT_CLOCK_OUT=hh T = t.txt
	while read t; do
		z=$(gmt math -Q -fi0T -fo0t $t 2020-01-01T SUB 1 ADD =)
		gmt grdmath -R-10/10/-10/10 -I1d 0 0 SDIST KM2DEG $z 0.25 MUL HYPOT $z DIV 4 POW -0.5 MUL EXP 5 MUL 8 $z SUB ADD = tmp_$z.grd
	done < t.txt
	gmt grdinterpolate -Zt.txt tmp_?.grd -Gfake_geot_cube.nc
	gmt makecpt -T0/12
	gmt subplot begin 4x2 -Fs5c -Scb -Srl -Blrbt -A1+gwhite -X5c -Y4c -M1c/0.25c -R-10/10/-10/10 -JM5c
		while read t; do
			z=$(gmt math -Q -fi0T -fo0t $t =)
			n=$(gmt math -Q -fi0T -fo0t $t 2020-01-01T SUB 1 ADD =)
			gmt subplot set -A$t
			#gmt grdimage "tmp_${n}.grd"
			gmt grdimage "fake_geot_cube.nc?(${z})"
		done < t.txt
	gmt subplot end
	gmt colorbar -DJBC -B
gmt end show
#rm -f tmp_?.grd
