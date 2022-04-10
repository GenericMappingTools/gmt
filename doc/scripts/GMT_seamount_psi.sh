#!/usr/bin/env bash
# Illustrate the effect of function psi on slides
gmt begin GMT_seamount_psi
	gmt set GMT_THEME cookbook
	gmt set FONT_ANNOT_PRIMARY 9p,Times-Italic FONT_LABEL 12p,Times-Italic
	gmt basemap -R0/1/0/1 -JX6i/1i -Bxafg0.5+l"@~t@~" -Byafg0.5+l"@~y(t)@~"
	beta=1;
	gmt math -T0/1/0.001 T ABS $beta POW = | gmt plot -W1p,red -l"@~b@~ = ${beta}"+jBR
	beta=4
	gmt math -T0/1/0.001 T ABS $beta POW = | gmt plot -W1p,green -l"@~b@~ = ${beta}"
	beta=0.25
	gmt math -T0/1/0.001 T ABS $beta POW = | gmt plot -W1p,blue -l"@~b@~ = ${beta}"
gmt end show
