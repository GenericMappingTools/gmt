#!/usr/bin/env bash
gmt begin GMT_linear
	gmt plot -R0/100/0/10 -JX3i/1.5i -Bag -BWSne+gsnow -Wthick,blue,- sqrt.txt
	gmt plot -St0.1i -N -Gred -Wfaint sqrt10.txt
gmt end show
