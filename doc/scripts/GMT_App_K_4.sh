#!/usr/bin/env bash
gmt begin GMT_App_K_4
	gmt coast -R-100/100/-100/100+uk -JE130.35/-0.2/3.5i -Dh -A1 \
		-Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B30mg10m -BWSne
	echo 130.35 -0.2 | gmt plot -SJ-40 -Wthicker
gmt end show
