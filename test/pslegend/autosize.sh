#!/usr/bin/env bash
#
# Testing gmt pslegend autosizing of symbols when nothing is specified
# other than the primary annotation font size.
# DVC_TEST

gmt begin autosize
	gmt set FONT_ANNOT_PRIMARY 18p

	cat > leg.txt <<- EOF
	# S dx1 symbol size fill pen [ dx2 text ]
	H 16p,Times-Roman Auto-sized at 18p
	D 3p 1p
	S - A - red 0.5p - Star
	S - C - red 0.5p - Circle
	S - D - red 0.5p - Diamond
	S - G - red 0.5p - Octagon
	S - H - red 0.5p - Hexagon
	S - I - red 0.5p - Inv triangle
	S - N - red 0.5p - Pentagon
	S - S - red 0.5p - Square
	S - T - red 0.5p - Triangle
	S - e - red 0.5p - Ellipse
	S - r - red 0.5p - Rectangle
	S - R - red 0.5p - Roundtangle
	S - j - red 0.5p - Rotangle
	S - w - red 0.5p - Wedge
	S - - - -   0.5p - Line
	S - f - red 0.5p - Front
	S - v - red 0.5p - Vector
	S - m - red 0.5p - Math angle
	S - k@sunglasses - - - - EPS
	S - k@volcano - red 0.5p - Macro
	EOF
	# Plot colored and outlined on left and just outline or right.
	gmt pslegend -R0/5/0/7.5 -JM12c -DjTL+l1.2+w5.5c+o0.3c -C0.2c -F+p+d -Baf leg.txt
	sed -e 's/red/-/g' leg.txt | gmt pslegend -DjTR+l1.2+w5.5c+o0.3c -C0.2c -F+p+d -Baf
gmt end show
