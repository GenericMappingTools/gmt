#!/usr/bin/env bash
#
# Demonstrate deprecated GMT4 vector
gmt begin GMT_vector4
gmt set GMT_THEME cookbook
# Cartesian straight arrows
	a=$(gmt math -Q 1 4 DIV ATAN R2D =)
	L=$(gmt math -Q 1 4 HYPOT =)
	gmt plot -R0/5/0/5 -JX5i -Sv0.4c/1.5c/0.8c -W3p,red -Gyellow --MAP_VECTOR_SHAPE=0.5 <<- EOF
	0.5	0.5	$a ${L}i
	EOF
	gmt plot -W0.25p,dashed -L <<- EOF
	> dxdy
	0.5	1.5
	0.5	0.5
	4.5	0.5
	4.5	1.5
	>
	0.5	0.5
	4.5	1.5
	EOF
	gmt plot -Sc0.2c -Gwhite -Wfaint <<- EOF
	0.5	0.5
	4.5	1.5
	EOF
	a1=$(gmt math -Q 180 25 SUB $a ADD =)
	a2=$(gmt math -Q 180 25 ADD $a ADD =)
	gmt plot -Sm0.2i -Gblack -W0.5p <<- EOF
	0.5 0.5 1.4i 0 $a
	0.5 0.5 0.75i $a 90
	EOF
	# Coordinates of line labeled h_l:
	xh1=$(gmt math -Q 0.5 ${L} 1.5 2.54 DIV SUB $a COSD MUL ADD 1 2.54 DIV $a SIND MUL SUB =)
	yh1=$(gmt math -Q 0.5 ${L} 1.5 2.54 DIV SUB $a SIND MUL ADD 1 2.54 DIV $a COSD MUL ADD =)
	xh2=$(gmt math -Q $xh1 1.5 2.54 DIV $a COSD MUL ADD =)
	yh2=$(gmt math -Q $yh1 1.5 2.54 DIV $a SIND MUL ADD =)
	# Coordinates of line labeled h_w
	xt1=$(gmt math -Q 0.5 ${L} 0.1 SUB 1.5 2.54 DIV SUB $a COSD MUL ADD 0.8 2.54 DIV $a SIND MUL SUB =)
	yt1=$(gmt math -Q 0.5 ${L} 0.1 SUB 1.5 2.54 DIV SUB $a SIND MUL ADD 0.8 2.54 DIV $a COSD MUL ADD =)
	xt2=$(gmt math -Q 0.5 ${L} 0.1 SUB 1.5 2.54 DIV SUB $a COSD MUL ADD =)
	yt2=$(gmt math -Q 0.5 ${L} 0.1 SUB 1.5 2.54 DIV SUB $a SIND MUL ADD =)
	# Coordinates of line labeled t_w
	xw1=$(gmt math -Q 0.5 ${L} 0.7 MUL $a COSD MUL ADD 0.2 2.54 DIV $a SIND MUL SUB =)
	yw1=$(gmt math -Q 0.5 ${L} 0.7 MUL $a SIND MUL ADD 0.2 2.54 DIV $a COSD MUL ADD =)
	xw2=$(gmt math -Q 0.5 ${L} 0.7 MUL $a COSD MUL ADD 0.2 2.54 DIV $a SIND MUL ADD =)
	yw2=$(gmt math -Q 0.5 ${L} 0.7 MUL $a SIND MUL ADD 0.2 2.54 DIV $a COSD MUL SUB =)
	gmt plot -W1p,blue <<- EOF
	> head length
	$xh1	$yh1
	$xh2	$yh2
	> head width
	$xt1	$yt1
	$xt2	$yt2
	> tail width
	$xw1	$yw1
	$xw2	$yw2
	EOF
	gmt text -F+f18p,Times-Italic+a+j -Dj6p -N -Gwhite -C0/0 <<- EOF
	4.5	1.5 0	LB (x@-e@-, y@-e@-)
	0.5	0.5 0	RT (x@-b@-, y@-b@-)
	1.75 0.6 0	CM d
	0.75 0.9 0	CM @~a@~
	2.5 1.1 $a 	CB L
	4.1 1.85 $a 	CB h@-l@-
	3.65 1.4 $a 	CB h@-w@-
	3.4 0.85 $a 	CB t@-w@-
	EOF
	gmt text -F+f18p,Helvetica-Bold -D0/-8p -N <<- EOF
	0.35 0.7	b
	4.6 1.5 e
	EOF
gmt end show
