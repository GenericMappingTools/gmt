#!/usr/bin/env bash
#
# Plot the earth masks

gmt begin GMT_earthmask ps
	gmt set GMT_THEME cookbook
	gmt makecpt -Clightblue,burlywood,blue,red,white -T-0.5/4.5/1 -N
	gmt grdimage @earth_mask_05m -JQ0/15c -Baf -BWsNe
	gmt legend -DJBC+w15c+o0/0.5c -F+p1p <<- EOF
	N 5
	S - s 0.5c lightblue 0.25p - Ocean [0]
	S - s 0.5c burlywood 0.25p - Land [1]
	S - s 0.5c blue 0.25p - Lake [2]
	S - s 0.5c red 0.25p - Island [3]
	S - s 0.5c white 0.25p - Pond [4]
	EOF
gmt end show
