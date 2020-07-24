#!/usr/bin/env bash
# Plot 5 life-stages in growing volcanoes in crossection
# Using un-truncated Gaussian shapes with linear flux
gmt begin GMT_seamount_cum_inc ps
	echo "100	75	50	5000	1	0" > t.txt
	gmt grdseamount -R40/160/74/76+uk -I100 -Gsmtc_%05.2f.nc t.txt -T0.8/0/0.2 -Qc/l -Dk -Cg -Mc.lis
	gmt grdseamount -R40/160/74/76+uk -I100 -Gsmti_%05.2f.nc t.txt -T0.8/0/0.2 -Qi/l -Dk -Cg -Mi.lis
	gmt subplot begin 5x2 -SCb+tc -SRl -A -R40/160/0/5.010 -Fs3i/0.5i -M8p
	gmt set FONT_TAG 16p,Times-Italic,black
	row=4
	while read t file rest; do
		gmt subplot set ${row},0 -A"t = $row"
		gmt grdtrack -G${file} -E40000/75000/160000/75000 -o0,2 | gmt plot -W1p -i0+s0.001,1+s0.001 -L+y0 -Gblack
		let row--
	done < c.lis
	row=4
	while read t file rest; do
		gmt subplot set ${row},1 -A"t = $row"
		gmt grdtrack -G${file} -E40000/75000/160000/75000 -o0,2 | gmt plot -W1p -i0+s0.001,1+s0.001 -L+y0 -Gblack
		let row--
	done < i.lis
	gmt subplot end
	rm -f smt[ci]*.nc ?.lis t.txt
gmt end show
