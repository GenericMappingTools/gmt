#!/usr/bin/env bash
# Testing vertical cuts of a cube via grdcut
#
gmt begin vert_cube_cuts
	# Create the 3-D cube from w = 5 * exp (-0.5*(r/z)^4) + 8 - z
	gmt math -T1/8/1 -o0 --FORMAT_FLOAT_OUT=%01.0f T = z.txt
	while read z; do
		gmt grdmath -R-10/10/-10/10 -I1 0 0 CDIST $z 0.25 MUL HYPOT $z DIV 4 POW -0.5 MUL EXP 5 MUL 8 $z SUB ADD = tmp_$z.grd
	done < z.txt
	gmt grdinterpolate -Zz.txt tmp_?.grd -Gfake_xyz_cube.nc
	# Cut slices at x = 0 and y = 6
	gmt grdcut fake_xyz_cube.nc -Ex0 -Gy.grd
	gmt grdcut fake_xyz_cube.nc -Ey6 -Gx.grd
	# Compute the yx and xz vertical grids directly using -R with x|y and z
	gmt grdmath -R-10/10/1/8 -I1 X Y 0.25 MUL HYPOT Y DIV 4 POW -0.5 MUL EXP 5 MUL 8 Y SUB ADD = orig_y.grd
	gmt grdmath -R-10/10/1/8 -I1 X 6 HYPOT Y 0.25 MUL HYPOT Y DIV 4 POW -0.5 MUL EXP 5 MUL 8 Y SUB ADD = orig_x.grd
	gmt makecpt -T0/12
	gmt subplot begin 2x2 -Fs8c -Sct+tc -Srl -Blrbt -A1+gwhite -R-10/10/1/8 -JX10c/-5c -M0.5c -T"Cuts of @[5 e^{\left [-\frac{1}{2}\left( \frac{r}{z}\right)^4\right]} + 8 - z@["
		gmt grdimage orig_x.grd -c -B+t"y-z cut x = 0"
		gmt grdimage orig_y.grd -c -B+t"x-z cut y = 6"
		gmt grdimage x.grd -c
		gmt grdimage y.grd -c
	gmt subplot end
	gmt colorbar -DJBC -B
gmt end show
