#!/usr/bin/env bash
# Testing simple LaTeX expression in pstext with font color
gmt begin latexrgb ps
	echo "8 4 <math>\Delta\sigma=x^2+y^2</math>" | gmt text -R0/16/0/8 -Jx1c -Baf -F+f48p,1,red
gmt end show
