#!/usr/bin/env bash
gmt begin GMT_linear
	gmt plot -R0/100/0/10 -JX8c/4c -Bag -BWSne+gsnow -Wthick,blue,- sqrt.txt
	gmt plot -St0.3c -N -Gred -Wfaint sqrt10.txt
gmt end show
