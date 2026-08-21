#!/usr/bin/env bash
# Test that scales and inverse scales given to grdvector work the same for all units
# Create a data set with a unit vector data length and then plot that to give the
# plot lengths of 1 inch, 2.5 cm, or 72 points via direct or inverse scales.  These
# should all be about 2.5-2.54 cm long regardless of how we specified the scales.
arg="-Q1c+e -Wthickest,black -Gred"
gmt begin vec_scales ps
  gmt set MAP_FRAME_TYPE plain
  echo 0 0 90 | gmt xyz2grd -R-3/3/-3/3 -I3 -Ga.nc
  echo 0 0 1  | gmt xyz2grd -R-3/3/-3/3 -I3 -Gr.nc
  gmt subplot begin 3x2 -Fs6c -Sct -Srl -Bafg1 -M1c -R-3/3/-3/3 -JX6c -A+jBR+o0.5c+gwhite+p1p
    gmt subplot set 0 -A"-S1i"					# 1 data unit per inch = 1 inch per data unit = 1 inch length
    gmt grdvector r.nc a.nc -A -S1i $arg
    gmt subplot set 1 -A"-Si1i"					# 1 inch per data unit = 1 inch length
    gmt grdvector r.nc a.nc -A -Si1i $arg
    gmt subplot set 2 -A"-S0.4c"				# 0.4 data unit per cm = 2.5 cm per data unit ~ 1 inch length
    gmt grdvector r.nc a.nc -A -S0.4c $arg
    gmt subplot set 3 -A"-Si2.5c"				# 2.5 cm per data unit ~ 1 inch length
    gmt grdvector r.nc a.nc -A -Si2.5c $arg
    gmt subplot set 4 -A"-S0.01388p"			# 0.01388 data unit per point = 72 points per data unit = 1 inch length
    gmt grdvector r.nc a.nc -A -S0.01388p $arg
    gmt subplot set 5  -A"-S72p"				# 72 points per data unit = 1 inch length
    gmt grdvector r.nc a.nc -A -Si72p $arg
 gmt subplot end
gmt end show
