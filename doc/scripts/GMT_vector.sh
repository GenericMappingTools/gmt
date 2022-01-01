#!/usr/bin/env bash
#
# Demonstrate vector details
gmt begin GMT_vector ps
gmt set GMT_THEME cookbook
# Cartesian straight arrows
	gmt plot -R0/5/0/5 -JX6i -Sv1i+s+e+a50+p0.25p,dashed -W5p <<- EOF
	0.5	0.5	4.5	1.5
	EOF
	gmt plot -Sv1i+s+e+a50+p2p,orange+h0.5 -W5p -Glightblue <<- EOF
	0.5	0.5	4.5	1.5
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
	a=$(gmt math -Q 1 4 DIV ATAN R2D =)
	a1=$(gmt math -Q 180 25 SUB $a ADD =)
	a2=$(gmt math -Q 180 25 ADD $a ADD =)
	gmt plot -Sm0.2i -Gblack -W0.5p <<- EOF
	0.5 0.5 1.1i 0 $a
	0.5 0.5 0.75i $a 90
	4.5 1.5 0.5i $a1 $a2
	EOF
	gmt text -F+f18p,Times-Italic+a+j -Dj6p -N <<- EOF
	4.5	0.5 0	TC @~D@~x
	0.5	1.5 0	RM @~D@~y
	4.5	1.5 0	LB (x@-e@-, y@-e@-)
	0.5	0.5 0	RT (x@-b@-, y@-b@-)
	1.3 0.6 0	CM d
	0.75 0.9 0	CM @~a@~
	4.20 1.43	0	CM @~q@~
	2.5 1.0 $a 	CB L
	EOF
	gmt text -F+f18p,Helvetica-Bold -D0/-8p -N <<- EOF
	0.35 0.7	b
	4.6 1.5 e
	EOF
gmt end show
