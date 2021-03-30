#!/usr/bin/env bash
#
# Makes a plot of the general movie dimensions
#
gmt begin GMT_movie_canvas ps
	gmt set GMT_THEME cookbook
	gmt basemap -R0/24/0/13.5 -Jx1c -B0
	gmt plot -W0.5p,- <<- EOF
	>
	2.5	0
	2.5	13.5
	>
	0	2.5
	24	2.5
	EOF
	gmt plot -Glightgreen -W1p <<- EOF
	2.5	2.5
	23	2.5
	23	12
	2.5	12
	EOF
	gmt plot -Sv24p+b+e+h0.5+s -Gblack -W2p <<- EOF
	0	1.25	2.5	1.25
	12	0	12	2.5
	0	6.75	24	6.75
	5	0	5	13.5
	EOF
	gmt text -F+f16p,Helvetica-Bold+a+j -Gwhite -W0.5p <<- EOF
	12	6.75	0	CM	MOVIE_WIDTH
	5	6.75	90	CM	MOVIE_HEIGHT
	2.8	1.25	0	LM	-X@%2%off@%%
	12	2.8		0	CB	-Y@%2%off@%%
	EOF
gmt end show
