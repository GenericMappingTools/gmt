#!/usr/bin/env bash
# Show what (x,y,p,q,r,s) in project are for man page
gmt begin project_setup
	a=$(gmt math -Q 4 2.5 DIV ATAN R2D =)
	X=(`echo 0 0 | gmt project -C0/-1 -E2.5/3 -N`)
	gmt plot -JX4i/0 -R-3.5/4/-2.7/2.6 -Glightgray -B -Bx+l"@%7%x@%% or @%7%r@%%" -By+l"@%7%y@%% or @%7%s@%%" <<- EOF
	-1.5	-1.0625
	0	-2
	2	1.2
	0.5	2.1375
	EOF
	gmt plot -Sc0.3c -Gorange -Bg10 <<- EOF
	0	-1
	2	2.2
	EOF
	gmt plot -Sv16p+e+h1+s -W2p -Gblack <<- EOF
	0	-1	2	2.2
	0	-1	-2.5	0.5625
	EOF
	echo 0 0 | gmt plot -Sc0.3c -Gred
	# Get coordinates of the (0,q) point as well so we can dash the line
	x=$(gmt math -Q ${X[3]} $a SIND MUL NEG =)
	y=$(gmt math -Q ${X[3]} $a COSD MUL 1 SUB =)
	echo ${X[4]} ${X[5]} | gmt plot -Sc0.2c -Gblue
	gmt text -F+f12p,Times-Italic+a+j -Dj0.15c <<- EOF
	0		-1			0		TL @%7%C@%%
	2		2.2			0		BR @%7%E@%%
	1.9		1.9			${a}	TC	p
	-2.3	0.4			${a}	RM	q
	2		1.2			${a}	TC	L@-max@-
	0		-2			${a}	TC	L@-min@-
	0		-2			${a}	RB	W@-min@-
	-1.5	-1.0625		${a}	RM	W@-max@-
	0.45	0.8			-16		TC	@~a@~
	EOF
	echo 0 0 ${X[4]} ${X[5]} | gmt plot -Sv12p+s -W0.25p,red,-
	echo 0 0 ${x} ${y} | gmt plot -Sv12p+s -W0.25p,red,-
	echo 0 -1 1i ${a} 90 | gmt plot -Sm9p+b -W0.5p -Gblack
gmt end show
