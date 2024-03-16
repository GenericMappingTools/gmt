#!/usr/bin/env bash
gmt begin GMT_linear
	gmt set GMT_THEME cookbook
	gmt math -T0/100/1 T SQRT = sqrt.txt
	gmt math -T0/100/10 T SQRT = sqrt10.txt
	gmt plot -R0/100/0/10 -JX8c/4c -Bag -BWSne+gsnow -Wthick,blue,- sqrt.txt
	gmt plot -St0.3c -N -Gred -Wfaint sqrt10.txt
gmt end show
