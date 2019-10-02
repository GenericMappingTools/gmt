#!/usr/bin/env bash
gmt begin GMT_App_K_4
	gmt coast -Rk-100/100/-100/100 -JE130.35/-0.2/3.5i -Dh -A1 \
	  -Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B30mg10m -BWSne 
	echo 130.35 -0.2 | gmt psxy -SJ-40 -Wthicker
gmt end show
