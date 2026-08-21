#!/usr/bin/env bash
#		GMT EXAMPLE 30
#
# Purpose:	Show graph mode and math angles
# GMT modules:	gmtmath, basemap, text and plot
# Unix progs:	echo, rm
#
# Draw generic x-y axes with arrows
gmt begin ex30
	gmt basemap -R0/360/-1.25/1.75 -JX20c/15c -Bx90f30+u@. -By1g10 -BWS+t"Two Trigonometric Functions" \
		--MAP_FRAME_TYPE=graph --MAP_VECTOR_SHAPE=0.5

	# Draw sine an cosine curves
	gmt math -T0/360/0.1 T COSD = | gmt plot -W3p
	gmt math -T0/360/0.1 T SIND = | gmt plot -W3p,0_6 --PS_LINE_CAP=round

	# Indicate the x-angle = 120 degrees
	gmt plot -W0.5p,- <<- EOF
	120	-1.25
	120	1.25
	EOF

	gmt text -Dj0.2c -N -F+f+j <<- EOF
	360 1 18p,Times-Roman RB x = cos(@%12%a@%%)
	360 0 18p,Times-Roman RB y = sin(@%12%a@%%)
	120 -1.25 14p,Times-Roman LB 120@.
	370 -1.35 24p,Symbol LT a
	-5 1.85 24p,Times-Roman RT x,y
	EOF

	# Draw a circle and indicate the 0-70 degree angle
	echo 0 0 | gmt plot -R-1/1/-1/1 -Jx3.8c -X9c -Y7c -Sc5c -W1p -N
	gmt plot -W1p <<- EOF
	> x-gridline  -Wdefault
	-1	0
	1	0
	> y-gridline  -Wdefault
	0	-1
	0	1
	> angle = 0
	0	0
	1	0
	> angle = 120
	0	0
	-0.5	0.866025
	> x-gmt projection -W2p
	-0.3333	0
	0	0
	> y-gmt projection -W2p
	-0.3333	0.57735
	-0.3333	0
	EOF

	gmt text -Dj4p -F+f+a+j <<- EOF
	-0.16666 0 12p,Times-Roman 0 CT x
	-0.3333 0.2888675 12p,Times-Roman 0 RM y
	0.22 0.27 12p,Symbol -30 CB a
	-0.33333 0.6 12p,Times-Roman 30 LB 120@.
	EOF

	echo 0 0 1.25c 0 120 | gmt plot -Sm0.4c+e -W1p -Gblack
gmt end show
