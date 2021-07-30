#!/usr/bin/env bash
# Show what (x,y,p,q,r,s) in project are for man page
gmt begin project_setup ps
	a=$(gmt math -Q 4 2.5 DIV ATAN R2D =)
	X=(`echo 0 0 | gmt project -C0/-1 -E2.5/3 -N`)
	gmt subplot begin 1x1 -Fs4i -R-3.5/4/-2.7/2.6 -Scb+l"@%7%x@%% or @%7%r@%%" -Srl+l"@%7%y@%% or @%7%s@%%" -B
	gmt basemap -Jx? -c
	gmt plot -Glightgray <<- EOF
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
	p=${X[2]}
	q=${X[3]}
	r1=${X[4]}
	s1=${X[5]}
	# Get coordinates of the (0,q) point as well so we can dash the line
	r2=$(gmt math -Q $q $a SIND MUL NEG =)
	s2=$(gmt math -Q $q $a COSD MUL 1 SUB =)
	echo $r1 $s1 | gmt plot -Sc0.2c -Gblue
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
	echo 0 0 $r1 $s1 | gmt plot -Sv12p+s -W0.25p,red,-
	echo 0 0 $r2 $s2 | gmt plot -Sv12p+s -W0.25p,red,-
	echo 0 -1 1i ${a} 90 | gmt plot -Sm9p+b -W0.5p -Gblack
	gmt subplot end
gmt end show
