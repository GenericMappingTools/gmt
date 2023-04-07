#!/usr/bin/env bash
# Plot 5 life-stages in growing volcanoes in crossection
# Using un-truncated Gaussian shapes with Gaussian flux
gmt begin GMT_seamount_cum_inc
	gmt set GMT_THEME cookbook
	echo "100	75	50	5000	1	0" > t.txt
	gmt grdseamount -R40/160/74/76+uk -I100 -Gsmtc_%05.2f.nc t.txt -T0.8/0/0.2 -Qc/g -Dk -Cg -Mc.lis
	gmt grdseamount -R40/160/74/76+uk -I100 -Gsmti_%05.2f.nc t.txt -T0.8/0/0.2 -Qi/g -Dk -Cg -Mi.lis
	gmt subplot begin 5x2 -Scb+tc -Srl -A -R40/160/0/5.010 -Fs3i/0.5i -M8p
		gmt set FONT_TAG 16p,Times-Italic,black
		gmt subplot set 0,0 -A"t = 4"
		gmt grdtrack -Gsmtc_00.00.nc -E40000/75000/160000/75000 -o0,2 | gmt plot -W1p -i0+s0.001,1+s0.001 -L+y0 -Gblack
		gmt subplot set 1,0 -A"t = 3"
		gmt grdtrack -Gsmtc_00.20.nc -E40000/75000/160000/75000 -o0,2 | gmt plot -W1p -i0+s0.001,1+s0.001 -L+y0 -Gblack
		gmt subplot set 2,0 -A"t = 2"
		gmt grdtrack -Gsmtc_00.40.nc -E40000/75000/160000/75000 -o0,2 | gmt plot -W1p -i0+s0.001,1+s0.001 -L+y0 -Gblack
		gmt subplot set 3,0 -A"t = 1"
		gmt grdtrack -Gsmtc_00.60.nc -E40000/75000/160000/75000 -o0,2 | gmt plot -W1p -i0+s0.001,1+s0.001 -L+y0 -Gblack
		gmt subplot set 4,0 -A"t = 0"
		gmt grdtrack -Gsmtc_00.80.nc -E40000/75000/160000/75000 -o0,2 | gmt plot -W1p -i0+s0.001,1+s0.001 -L+y0 -Gblack
		#
		gmt subplot set 0,1 -A"t = 4"
		gmt grdtrack -Gsmti_00.00.nc -E40000/75000/160000/75000 -o0,2 | gmt plot -W1p -i0+s0.001,1+s0.001 -L+y0 -Gblack
		gmt subplot set 1,1 -A"t = 3"
		gmt grdtrack -Gsmti_00.20.nc -E40000/75000/160000/75000 -o0,2 | gmt plot -W1p -i0+s0.001,1+s0.001 -L+y0 -Gblack
		gmt subplot set 2,1 -A"t = 2"
		gmt grdtrack -Gsmti_00.40.nc -E40000/75000/160000/75000 -o0,2 | gmt plot -W1p -i0+s0.001,1+s0.001 -L+y0 -Gblack
		gmt subplot set 3,1 -A"t = 1"
		gmt grdtrack -Gsmti_00.60.nc -E40000/75000/160000/75000 -o0,2 | gmt plot -W1p -i0+s0.001,1+s0.001 -L+y0 -Gblack
		gmt subplot set 4,1 -A"t = 0"
		gmt grdtrack -Gsmti_00.80.nc -E40000/75000/160000/75000 -o0,2 | gmt plot -W1p -i0+s0.001,1+s0.001 -L+y0 -Gblack
	gmt subplot end
gmt end show
rm -f smt[ci]*.nc ?.lis t.txt
