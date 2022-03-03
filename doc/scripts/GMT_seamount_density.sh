#!/usr/bin/env bash
# Show the seamount density model offered in grdseamount
gmt begin GMT_seamount_density
	echo 0 0 50 6000 | gmt grdseamount -H6000/2500/3000+p1+d200 -Kmodel.grd -Cg
	gmt makecpt -Chot -I -T2500/3000 --COLOR_NAN=white
	# Plot density reference model above
	gmt grdimage model.grd -R0/1/0/1 -JX15c/4c -Bxafg1+l"Normalized radial distance, @%6%r@%%" -By+l"Normalized height, @%6%h(r)@%%"
	gmt math -T0/1/0.01 T 2 POW -4.5 MUL EXP = | gmt plot -W0.25p
	z=$(gmt math -Q 0.4 2 POW -4.5 MUL EXP =)
	y=$(gmt math -Q 1 $z ADD 2 DIV =)
	echo 0.4 0.15 | gmt plot -Sc4p -Gwhite -W0.25p
	gmt plot -Sv12p+b+e+s -W1p -Gblack <<- EOF
	0.4	0.15	0.4	${z}
	0.4	1	0.4	${z}
	EOF
	printf "0.4	%s\n1 %s\n" $z $z | gmt plot -W0.25p,-
	printf "0.4	%s\n0.75	%s\n" $y $y | gmt plot -W0.25p
	printf "0.4	0.3\n0.75	0.3\n" | gmt plot -W0.25p
	printf "0.4	0\n0.4	0.15\n0.75	0.15\n" | gmt plot -W0.25p
	gmt text -F+f12p+jLM -Dj3p <<- EOF
	0.75	$y @[h_r - h(r)@[
	0.75	0.3 @[h(r) - z@[
	0.75	0.15 @[(r, z)@[
	EOF
	gmt text -F+f9p+j -Dj3p <<- EOF
	0.75	$z BL Outside seamount
	0.75	$z TL Inside seamount
	EOF
	gmt colorbar -DJBC+w80%/0.25c -Bx -By+l"kg m@+-3@+"
gmt end show
