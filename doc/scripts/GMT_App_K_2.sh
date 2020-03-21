#!/usr/bin/env bash
gmt begin GMT_App_K_2
	gmt coast -R-2000/2000/-2000/2000+uk -JE130.35/-0.2/3.5i -Dl -A100 \
		-Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B10g5 -BWSne
	echo 130.35 -0.2 | gmt plot -SJ-1000 -Wthicker
gmt end show
