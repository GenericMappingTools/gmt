#!/usr/bin/env bash
gmt begin GMT_App_K_2
	gmt coast -Rk-2000/2000/-2000/2000 -JE130.35/-0.2/3.5i -Dl -A100 \
	-Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B10g5 -BWSne 
	echo 130.35 -0.2 | gmt psxy -SJ-1000 -Wthicker
gmt end show
