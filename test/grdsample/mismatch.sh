#!/usr/bin/env bash
# Make sure the read region includes all nodes needed for sampling write region
# DVC_TEST
gmt begin mismatch ps
	gmt grdmath -R1/7/0/6 -I1 X Y MUL = t.grd
	gmt grdcontour t.grd -W0.25p,blue -C1 -Jx1i -Bafg1 -B+t"grdsample wesn_i (blue) and wesn_o (red) regions"
	gmt grd2xyz t.grd | gmt plot -Sc0.25c -Gblue -N
	gmt grdmath -R2.55/5.19/1.1/3.2 -I0.33/0.42 X Y MUL = t.grd
	gmt grdcontour t.grd -C1 -W0.5p,red
	gmt grd2xyz t.grd | gmt plot -Ss0.1c -Gred
	gmt grdinfo t.grd -Ib | gmt plot -W0.25p,red
	gmt plot -W2p,blue -L <<- EOF
	2	1
	6	1
	6	4
	2	4
	EOF
	gmt text -F+f16p -Gwhite -D0/0.5c <<- EOF
	4 4 Subset R = 2/6/1/4 I = 1
	3.87 3.2 R = 2.55/5.19/1.1/3.2 I = 0.33/0.42
	EOF
gmt end show
