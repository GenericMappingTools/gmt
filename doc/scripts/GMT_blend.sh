#!/usr/bin/env bash
# Demonstrate blending and weights
# Large grid has no inner boundary
# Crossection shows output values are correct
gmt begin GMT_blend
	gmt set GMT_THEME cookbook
	gmt grdmath -R0/100/0/45  -I0.2 1 = 1.grd
	gmt grdmath -R10/60/15/40 -I0.2 2 = 2.grd
	gmt grdmath -R45/75/5/35  -I0.2 3 = 3.grd
	cat <<- EOF > blend.lis
	1.grd	1
	2.grd	25/55/20/33	2
	3.grd	48/70/7/25	4
	EOF
	gmt grdblend blend.lis -R1.grd -Gblend.grd
	cat <<- EOF > t.cpt
	1	lightyellow	2	lightorange
	2	lightorange	3	lightred
	EOF
	gmt grdimage blend.grd -Jx0.2c -B -Ct.cpt
	gmt grdinfo -Ib 2.grd | gmt plot -W2p
	gmt grdinfo -Ib 3.grd | gmt plot -W2p
	gmt plot -W0.5,- -L <<- EOF
	> Inner region for grid 2
	25	20
	55	20
	55	35
	25	35
	> Inner region for grid 3
	48	7
	70	7
	70	25
	48	25
	EOF
	printf "0	22\n100	22\n" | gmt plot -W1p,blue
	gmt text -F+f12p,Times-Italic+j -Dj4p <<- EOF
	0	45 TL z = 1, w = 1
	60  40 TR z = 2, w = 2
	75  35 TR z = 3, w = 4
	EOF
	gmt colorbar -DjRM -F+gwhite+p0.25p -Bx -By+lz
	gmt grdtrack -Gblend.grd -E0/22/100/22 -o0,2 | gmt plot -R0/100/0.8/2.8 -Jx0.2c/1c -Bya1g1 -BWbrt -Y9.2c -W1p,blue
gmt end show
