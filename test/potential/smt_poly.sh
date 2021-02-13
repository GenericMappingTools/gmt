#!/usr/bin/env bash
# Show the polynomial seamount shape, compare it to Gaussian
gmt begin smt_poly ps
	echo "0	0      90	95	60      1000" > t.txt
	gmt grdseamount t.txt -R-100/100/-100/100 -I1 -Co -r -Go.nc -E
	gmt grdseamount t.txt -R-100/100/-100/100 -I1 -Cg -r -Gg.nc -E
	gmt grdcontour o.nc -JX15c -B -BWSrt -C100 -A200 -GlCB/CT -Xc
	gmt grdtrack -Go.nc -ELM/RM -o0,2 > to.txt
	gmt grdtrack -Gg.nc -ELM/RM -o0,2 > tg.txt
	gmt plot -R-100/100/-100/1100 -JX15c/8c -W2p -Bafg1000 -BWSrt to.txt -Y16c -l"Polynomial"
	gmt plot -W0.5p,red tg.txt -l"Gaussian"
gmt end show
